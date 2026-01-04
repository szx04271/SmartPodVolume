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

namespace SmartPodVolumeWizard
{
    /// <summary>
    /// MainWindow.xaml 的交互逻辑
    /// </summary>
    public partial class MainWindow : Window
    {
        public const string RUNNING_TEXT = "运行中";
        public const string STOPPED_TEXT = "已停止";

        public static readonly Brush RUNNING_COLOR = new SolidColorBrush(Colors.DarkGreen);
        public static readonly Brush STOPPED_COLOR = new SolidColorBrush(Colors.Red);

        public const string START_TEXT = "启动服务";
        public const string STOP_TEXT = "停止服务";

        private bool isServiceRunning = false;

        public MainWindow()
        {
            InitializeComponent();

            bkgndServiceStatusLabel.Text = STOPPED_TEXT;
            bkgndServiceStatusLabel.Foreground = STOPPED_COLOR;
            startButton.Content = START_TEXT;
        }

        private void startButton_Click(object sender, RoutedEventArgs e)
        {
            if (isServiceRunning) {
                isServiceRunning = false;
                bkgndServiceStatusLabel.Text = STOPPED_TEXT;
                bkgndServiceStatusLabel.Foreground = STOPPED_COLOR;
                startButton.Content = START_TEXT;
            }
            else {
                isServiceRunning = true;
                bkgndServiceStatusLabel.Text = RUNNING_TEXT;
                bkgndServiceStatusLabel.Foreground = RUNNING_COLOR;
                startButton.Content = STOP_TEXT;
            }
        }
    }
}
