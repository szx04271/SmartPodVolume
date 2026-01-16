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

namespace SmartPodVolumeWizard
{
    using DeviceIdInfoPair = KeyValuePair<string, ConfigReadWrite.DeviceInfo>;
    using DeviceMap = Dictionary<string, ConfigReadWrite.DeviceInfo>;

    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        MainViewModel Vm => (MainViewModel)DataContext;
        
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
            this.Closing += MainWindow_Closing;
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

        private void MainWindow_Closing(object sender, System.ComponentModel.CancelEventArgs e)
        {
            if (IsConfigModified)
            {
                var msgBoxResult = MessageBox.Show("您已修改了设置，但未保存，是否立即保存并应用？",
                    "应用设置提示", MessageBoxButton.YesNoCancel, MessageBoxImage.Question);
                if (msgBoxResult == MessageBoxResult.Yes)
                {
                    // TODO: restart service
                }
                else if (msgBoxResult == MessageBoxResult.Cancel)
                {
                    e.Cancel = true;
                }
            }
        }

        private void StartButton_Click(object sender, RoutedEventArgs e)
        {
            IsServiceRunning = !IsServiceRunning;
            IsConfigModified = !IsConfigModified;
        }

        private void RefreshLists()
        {
            WhiteListDevices = ConfigReadWrite.GetWhiteListDevices();
            BlackListDevices = ConfigReadWrite.GetBlackListDevices();

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
        }

        private void ApplyConfigBtn_Click(object sender, RoutedEventArgs e)
        {
        }
    }
}
