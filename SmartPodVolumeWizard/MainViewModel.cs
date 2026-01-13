using System.ComponentModel;
using System.Runtime.CompilerServices;
using System.Collections.Immutable;

namespace SmartPodVolumeWizard
{
    public class MainViewModel : ViewModelBase
    {
        private readonly ImmutableArray<string> _myTexts = 
            ImmutableArray.Create(new[] { "移入黑名单", "接管该设备" });

        private bool _serviceRunning;
        private int _listTabSelectedIndex;
        private string _moveToOtherListBtnText;

        public bool ServiceRunning
        {
            get { return _serviceRunning; }
            set
            {
                if (_serviceRunning != value)
                {
                    _serviceRunning = value;
                    OnPropertyChanged();
                }
            }
        }

        public int ListTabSelectedIndex
        {
            get { return _listTabSelectedIndex; }
            set
            {
                if (_listTabSelectedIndex != value)
                {
                    _listTabSelectedIndex = value;
                    OnPropertyChanged();

                    OnListTabSelectedIndexChanged();
                }
            }
        }

        public string MoveToOtherListBtnText
        {
            get => _moveToOtherListBtnText;
            set
            {
                if (_moveToOtherListBtnText != value)
                {
                    _moveToOtherListBtnText = value;
                    OnPropertyChanged();
                }
            }
        }

        public void OnListTabSelectedIndexChanged()
        {
            MoveToOtherListBtnText = _myTexts[ListTabSelectedIndex];
        }

        public MainViewModel()
        {
            MoveToOtherListBtnText = _myTexts[0];
        }
    }
}
