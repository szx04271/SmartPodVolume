using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace SmartPodVolumeWizard
{
    public class NativeDarkThemer
    {
        const string DllName = "CommCtrlDarkThemer.dll";

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern long DarkThemer_InstallForCurrentThread();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern long DarkThemer_UninstallForCurrentThread();

        [DllImport(DllName, CallingConvention = CallingConvention.Cdecl)]
        public static extern ulong DarkThemer_ForceAppDark(byte ifDark);
    }
}
