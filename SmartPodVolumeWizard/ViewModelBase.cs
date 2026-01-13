// ViewModelBase.cs
// for autoupdate of UI elements' attributes when C# variables they binded to changes 

using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace SmartPodVolumeWizard
{
    public class ViewModelBase : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        protected void OnPropertyChanged([CallerMemberName] string name = null)
        {
            PropertyChanged?.Invoke(this, new PropertyChangedEventArgs(name));
        }
    }
}
