using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;
using System.Threading;
using Microsoft.Win32;
using System.Reflection;
using System.Diagnostics;
using System.IO.Pipes;
using System.Runtime.Remoting.Messaging;

namespace SmartPodVolumeWizard
{
    using DeviceIdInfoPair = KeyValuePair<string, ConfigReadWrite.DeviceInfo>;
    using DeviceMap = Dictionary<string, ConfigReadWrite.DeviceInfo>;

    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        const string BkServiceExeName = "SmartPodVolume.exe";
        const string AutoStartRegValueName = "SmartPodVolume";
        const string AutoStartRegKeyPath = @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run";

        MainViewModel Vm => (MainViewModel)DataContext;

        // volatile 使对该变量的赋值立即反映到内存中，防止子线程读到缓存值
        volatile bool _serviceStopRequired = false;
        volatile TaskCompletionSource<bool> _servStopCompletionSrc, _servStartCompletionSrc;

        CancellationTokenSource _cts = new CancellationTokenSource();
        Task _bkgndProcessWatcherTask = null;
        
        bool IsServiceRunning
        {
            get => Vm.ServiceRunning;
            set => Vm.ServiceRunning = value;
        }

        bool IsConfigModified
        {
            get => Vm.ConfigModified;
            set => Vm.ConfigModified = value;
        }

        DeviceMap WhiteListDevices
        {
            get => Vm.WhiteListDevices;
            set => Vm.WhiteListDevices = value;
        }
        
        DeviceMap BlackListDevices
        {
            get => Vm.BlackListDevices;
            set => Vm.BlackListDevices = value;
        }

        private int _updatesToBeIgnored = 0;

        public MainWindow()
        {
            InitializeComponent();
            Loaded += MainWindow_Loaded;
        }

        private void MainWindow_Loaded(object sender, RoutedEventArgs e)
        {
            // set working dir to where the exe is actually located
            Directory.SetCurrentDirectory(AppDomain.CurrentDomain.BaseDirectory);

            CheckBkServiceExeFileExistence();

            this.Closing += MainWindow_Closing;

            _bkgndProcessWatcherTask = Task.Run(() => BkgndProcessWatcher(_cts.Token) );

            ValidateAutoStartOption();
            RefreshLists();

            // watch config file change
            FileSystemWatcher watcher = new FileSystemWatcher
            {
                Path = Directory.GetCurrentDirectory(),
                Filter = ConfigReadWrite.ConfigFileName,
                EnableRaisingEvents = true, // what use?
                // without filter `FileName`, `Deleted` can't be triggered
                NotifyFilter = NotifyFilters.LastWrite | NotifyFilters.FileName
            };
            watcher.Created += ConfigFileChanged;
            watcher.Changed += ConfigFileChanged;
            watcher.Deleted += ConfigFileChanged;
            watcher.Renamed += ConfigFileChanged;
        }

        private void ConfigFileChanged(object sender, FileSystemEventArgs e)
        {
            // 这里不是主线程，注意不能直接操作UI
            try
            {
                if (e.ChangeType == WatcherChangeTypes.Changed || e.ChangeType == WatcherChangeTypes.Created)
                {
                    // Avoid executing the function body multiple times due to multiple buffered writing
                    // of one actual WriteFile call.
                    // If file is still being written, this will throw an error.
                    using (var file = File.OpenWrite(e.FullPath)) { }
                }

                if (e.ChangeType == WatcherChangeTypes.Changed)
                { 
                    // File.WriteAllText called by us will trigger at least one `Changed`
                    // whether the file existed or not before.
                    if (_updatesToBeIgnored != 0)
                    {
                        Interlocked.Decrement(ref _updatesToBeIgnored);
                        return;
                    }
                }

                Application.Current.Dispatcher.Invoke(() => { RefreshLists(); });
            }
            catch { }
        }

        private async void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (!Vm.CanStartOrEndService)
            {
                e.Cancel = true;
                return;
            }

            if (IsConfigModified)
            {
                var msgBoxResult = MessageBox.Show("您已修改了设置，但未保存，是否立即保存并应用？",
                    "应用设置提示", MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
                if (msgBoxResult == MessageBoxResult.Yes)
                {
                    e.Cancel = true;
                    await RestartService();
                    Close();
                }
                else if (msgBoxResult == MessageBoxResult.Cancel)
                {
                    e.Cancel = true;
                    return;
                }
            }

            _cts?.Cancel(); // 其实这里就算没这句，关闭程序后CLR也会帮我们取消。
                            // 但取消会导致子线程里面的耗时函数抛异常，catch住就好了。
            //_bkgndProcessWatcherTask.Wait();
        }

        private async Task StopService()
        {
            var src = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
            Interlocked.Exchange(ref _servStopCompletionSrc, src);

            _serviceStopRequired = true;
            // 若想更稳，可以再开个后台线程，每隔一秒检测两个TaskCompletionSource的值，
            // 如果不为null，则根据IsServiceRunning的值来触发其一，这样能彻底避免错过了
            // BkgndProcessWatcher 发的信号从而无限期等下去的await。
            await _servStopCompletionSrc.Task;
            _serviceStopRequired = false;

            Interlocked.Exchange(ref _servStopCompletionSrc, null);
        }

        private async Task StartService()
        {
            var src = new TaskCompletionSource<bool>(TaskCreationOptions.RunContinuationsAsynchronously);
            Interlocked.Exchange(ref _servStartCompletionSrc, src);

            try
            {
                Process.Start(BkServiceExeName);
            }
            catch (Exception ex)
            {
                MessageBox.Show($"启动后台进程失败（{ex.Message}）", "错误", MessageBoxButton.OK, MessageBoxImage.Error);
                return;
            }

            await _servStartCompletionSrc.Task;

            Interlocked.Exchange(ref _servStartCompletionSrc, null);
            // A start of service means that config has been reloaded.
            // So reset this sign.
            IsConfigModified = false;
        }

        private async void StartButton_Click(object sender, RoutedEventArgs e)
        {
            Vm.CanStartOrEndService = false;

            if (IsServiceRunning)
            {
                await StopService();
            }
            else
            {
                await StartService();
            }

            Vm.CanStartOrEndService = true;
        }

        private void RefreshLists()
        {
            var configJson = ConfigReadWrite.TryGetConfigJson();
            WhiteListDevices = ConfigReadWrite.GetWhiteListDevices(configJson);
            BlackListDevices = ConfigReadWrite.GetBlackListDevices(configJson);

            WhiteListView.Items.Refresh();
            BlackListView.Items.Refresh();
        }

        private void RefreshListsBtn_Click(object sender, RoutedEventArgs e)
        {
            RefreshLists();
        }
        
        private void ShowNoSelectionTip()
        {
            MessageBox.Show("当前未选中任何设备。", "提示", MessageBoxButton.OK, MessageBoxImage.Warning);
        }

        private void MoveToTheOtherListBtn_Click(object sender, RoutedEventArgs e)
        {
            var currentPage = ListViewTab.SelectedIndex;
            ListView fromListView, toListView;
            DeviceMap fromDict, toDict;
            if (currentPage == 0)
            {
                fromListView = WhiteListView;
                toListView = BlackListView;
                fromDict = WhiteListDevices;
                toDict = BlackListDevices;
            }
            else
            {
                fromListView = BlackListView;
                toListView = WhiteListView;
                fromDict = BlackListDevices;
                toDict = WhiteListDevices;
            }

            var selectedDevice = (DeviceIdInfoPair?)fromListView.SelectedItem;
            if (selectedDevice == null)
            {
                ShowNoSelectionTip();
                return;
            }

            fromDict.Remove(selectedDevice.Value.Key);
            toDict.Add(selectedDevice.Value.Key, selectedDevice.Value.Value);
            ConfigReadWrite.SaveConfig(WhiteListDevices, BlackListDevices);
            Interlocked.Increment(ref _updatesToBeIgnored);

            fromListView.Items.Refresh();
            toListView.Items.Refresh();

            IsConfigModified = true;
        }

        private void ForgetDeviceBtn_Click(object sender, RoutedEventArgs e)
        {
            ListView fromListView;
            DeviceMap fromDict;
            if (ListViewTab.SelectedIndex == 0)
            {
                fromListView = WhiteListView;
                fromDict = WhiteListDevices;
            }
            else
            {
                fromListView = BlackListView;
                fromDict = BlackListDevices;
            }

            var selectedDevice = (DeviceIdInfoPair?)fromListView.SelectedItem;
            if (selectedDevice == null)
            {
                ShowNoSelectionTip();
                return;
            }

            fromDict.Remove(selectedDevice.Value.Key);
            ConfigReadWrite.SaveConfig(WhiteListDevices, BlackListDevices);
            Interlocked.Increment(ref _updatesToBeIgnored);

            fromListView.Items.Refresh();

            IsConfigModified = true;
        }

        private async Task RestartService()
        {
            Vm.CanStartOrEndService = false;

            if (IsServiceRunning)
            {
                await StopService();
            }
            await StartService();

            Vm.CanStartOrEndService = true;
        }

        private async void ApplyConfigBtn_Click(object sender, RoutedEventArgs e)
        {
            await RestartService();
        }

        private void CheckBkServiceExeFileExistence()
        {
            if (!File.Exists(BkServiceExeName))
            {
                MessageBox.Show($"后台服务组件 {BkServiceExeName} 不存在。请确保程序组件齐全且在同一目录下。",
                    "组件缺失", MessageBoxButton.OK, MessageBoxImage.Error);
                Close();
            }
        }

        private void AutoStartCheckBox_Click(object sender, RoutedEventArgs e)
        {
            if (AutoStartCheckBox.IsChecked == true)
            {
                try
                {
                    RegistryKey regKey = Registry.CurrentUser.OpenSubKey(
                        @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run", true);
                    regKey.SetValue(AutoStartRegValueName, $"\"{Directory.GetCurrentDirectory().TrimEnd('\\')}\\{BkServiceExeName}\"");
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"添加开机自启动项失败（{ex.Message}）", "错误", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    AutoStartCheckBox.IsChecked = false;
                }
            }
            else
            {
                try
                {
                    RegistryKey regKey = Registry.CurrentUser.OpenSubKey(
                        @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run", true);
                    regKey.DeleteValue(AutoStartRegValueName);
                }
                catch (Exception ex)
                {
                    MessageBox.Show($"删除开机自启动项失败（{ex.Message}）", "错误", MessageBoxButton.OK, MessageBoxImage.Exclamation);
                    AutoStartCheckBox.IsChecked = true;
                }
            }
        }

        private void ValidateAutoStartOption()
        {
            try
            {
                RegistryKey regKey = Registry.CurrentUser.OpenSubKey(
                    @"SOFTWARE\Microsoft\Windows\CurrentVersion\Run", false);
                var value = regKey.GetValue(AutoStartRegValueName);
                AutoStartCheckBox.IsChecked = value != null;
            }
            catch { }
        }

        private async Task BkgndProcessWatcher(CancellationToken token)
        {
            const string BkgndProcessPipeName = "D9227EEB_62EB_4903_B4A1_5ACB5D97FCBC";
            const byte BkgndProcessAliveSignal = 0x84;
            const byte BkgndProcessStopSignal = 0xfe;

            // use named pipe for interprocess communication
            // wizard as server
            using (var pipeStream = new NamedPipeServerStream(BkgndProcessPipeName, PipeDirection.InOut,
                1, PipeTransmissionMode.Byte, PipeOptions.Asynchronous))
            {
                while (!token.IsCancellationRequested)
                {
                    try
                    {
                        await pipeStream.WaitForConnectionAsync(token);
                    }
                    catch 
                    {
                        // maybe should retry certain times here
                    }

                    if (token.IsCancellationRequested)
                    {
                        break;
                    }

                    // connected, that means, bkgnd process launched
                    _servStartCompletionSrc?.TrySetResult(true);
                    Application.Current.Dispatcher.Invoke(() => { IsServiceRunning = true; });

                    byte[] buffer = new byte[1];
                    while (!token.IsCancellationRequested)
                    {
                        try
                        {
                            var bytesRead = await pipeStream.ReadAsync(buffer, 0, buffer.Length, token);
                            if (bytesRead == buffer.Length)
                            {
                                if (buffer[0] == BkgndProcessAliveSignal)
                                {
                                    // bkgnd process still alive
                                    if (_serviceStopRequired)
                                    {
                                        buffer[0] = BkgndProcessStopSignal;
                                        pipeStream.Write(buffer, 0, buffer.Length); // ask bkgnd process to stop
                                    }

                                    continue;
                                }
                            }
                            else
                            {
                                // client disconnected
                                break;
                            }
                        }
                        catch { break; }
                    }
                    pipeStream.Disconnect(); // reset stream state, so that it can connect again

                    _servStopCompletionSrc?.TrySetResult(true);
                    Application.Current?.Dispatcher.Invoke(() => { IsServiceRunning = false; });
                }
            }
        }
    }
}
