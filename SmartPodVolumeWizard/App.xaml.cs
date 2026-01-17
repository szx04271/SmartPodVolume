using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;

namespace SmartPodVolumeWizard
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {
        const string InstanceMutexName = "Global\\5AB0950A_35DE_49D0_B8A2_2E1F16D4E035";

        Mutex _instanceMutex;

        public App()
        {
            this.Startup += App_Startup;
            this.Exit += App_Exit;
        }

        private void App_Exit(object sender, ExitEventArgs e)
        {
            throw new NotImplementedException();
        }

        private void App_Startup(object sender, StartupEventArgs e)
        {
            // limit to 1 instance
            _instanceMutex = new Mutex(true, InstanceMutexName, out bool createdNewMutex);
            if (!createdNewMutex)
            {
                _instanceMutex.Dispose();
                MessageBox.Show("已有一个设置向导在运行，请勿运行多个。", "SmartPodVolume 设置向导", MessageBoxButton.OK,
                    MessageBoxImage.Asterisk);
                Shutdown();
            }
        }
    }
}
