using Microsoft.Win32;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace SmartPodVolumeWizard
{
    public class ColorThemeGetter
    {
        private static bool Is1903OrGreater => Environment.OSVersion.Version.Build >= 18362;

        public static bool IsDarkModeEnabled()
        {
            if (!Is1903OrGreater)
            {
                return false;
            }

            const string registryKey = @"HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Themes\Personalize";
            const string registryValue = "AppsUseLightTheme";
            try
            {
                return (int)Registry.GetValue(registryKey, registryValue, 1) == 0;
            }
            catch
            {
                return false;
            }
        }
    }
}
