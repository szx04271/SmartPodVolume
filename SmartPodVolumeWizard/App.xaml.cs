using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media.Animation;

namespace SmartPodVolumeWizard
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {
        const string InstanceMutexName = "Global\\5AB0950A_35DE_49D0_B8A2_2E1F16D4E035";
        const string DarkThemeDictFileName = "Dark.xaml";
        const string LightThemeDictFileName = "Light.xaml";
        readonly Uri _darkThemeUri = new Uri("Themes/" + DarkThemeDictFileName, UriKind.Relative);
        readonly Uri _lightThemeUri = new Uri("Themes/" + LightThemeDictFileName, UriKind.Relative);

        Mutex _instanceMutex;
        bool _nativeDarkLoaded = false;

        public App()
        {
            this.Startup += App_Startup;
            this.Exit += App_Exit;
            SystemEvents.UserPreferenceChanged += SystemEvents_UserPreferenceChanged;
        }

        private void SystemEvents_UserPreferenceChanged(object sender, UserPreferenceChangedEventArgs e)
        {
            // "General" category is the one light/dark theme setting in
            if (e.Category == UserPreferenceCategory.General)
            {
                Current.Dispatcher.Invoke(() => ApplyTheme());
            }    
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

            ApplyTheme();
        }

        private void App_Exit(object sender, ExitEventArgs e)
        {
            if (_nativeDarkLoaded)
            {
                NativeDarkThemer.DarkThemer_UninstallForCurrentThread();
            }
        }

        private void ApplyTheme()
        {
            bool isDark = ColorThemeGetter.IsDarkModeEnabled();

            // 查找是否已经加载了浅色/深色主题
            // 注意：这里通过 Source 的字符串匹配来找，比较稳妥
            var existingLightDict = Current.Resources.MergedDictionaries
                .FirstOrDefault(d => d.Source != null && d.Source.OriginalString.EndsWith(LightThemeDictFileName));
            var existingDarkDict = Current.Resources.MergedDictionaries
                .FirstOrDefault(d => d.Source != null && d.Source.OriginalString.EndsWith(DarkThemeDictFileName));

            if (isDark)
            {
                if (!_nativeDarkLoaded)
                {
                    NativeDarkThemer.DarkThemer_ForceAppDark(1);
                    NativeDarkThemer.DarkThemer_InstallForCurrentThread();
                    _nativeDarkLoaded = true;
                }

                if (existingDarkDict == null)
                {
                    Current.Resources.MergedDictionaries.Add(new ResourceDictionary { Source = _darkThemeUri });
                }
                if (existingLightDict != null)
                {
                    Current.Resources.MergedDictionaries.Remove(existingLightDict);
                }
            }
            else
            {
                if (_nativeDarkLoaded)
                {
                    NativeDarkThemer.DarkThemer_ForceAppDark(0);
                    NativeDarkThemer.DarkThemer_UninstallForCurrentThread();
                    _nativeDarkLoaded = false;
                }

                if (existingLightDict == null)
                {
                    Current.Resources.MergedDictionaries.Add(new ResourceDictionary { Source = _lightThemeUri });
                }
                if (existingDarkDict != null)
                {
                    Current.Resources.MergedDictionaries.Remove(existingDarkDict);
                }
            }
        }
    }
}
