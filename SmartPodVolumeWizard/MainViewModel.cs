using System.Collections.Generic;
using System.Collections.Immutable;
using System.ComponentModel;
using System.Runtime.CompilerServices;

namespace SmartPodVolumeWizard
{
    using DeviceIdInfoPair = KeyValuePair<string, ConfigReadWrite.DeviceInfo>;
    using DeviceMap = Dictionary<string, ConfigReadWrite.DeviceInfo>;

    public class MainViewModel : ViewModelBase
    {
        private readonly ImmutableArray<string> _myTexts = 
            ImmutableArray.Create(new[] { "移入黑名单", "接管该设备" });

        private bool _serviceRunning;
        private int _listTabSelectedIndex;
        private string _moveToOtherListBtnText;
        private bool _configModified;
        private DeviceMap _whiteListDevices;
        private DeviceMap _blackListDevices;

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

        public bool ConfigModified
        {
            get => _configModified;
            set
            {
                if (_configModified != value)
                {
                    _configModified = value;
                    OnPropertyChanged();
                }
            }
        }

        public DeviceMap WhiteListDevices
        {
            get => _whiteListDevices;
            set
            {
                if (_whiteListDevices != value)
                {
                    _whiteListDevices = value;
                    OnPropertyChanged();
                }
            }
        }

        public DeviceMap BlackListDevices
        {
            get => _blackListDevices;
            set
            {
                if (_blackListDevices != value)
                {
                    _blackListDevices = value;
                    OnPropertyChanged();
                }
            }
        }

        public MainViewModel()
        {
            MoveToOtherListBtnText = _myTexts[0];
        }
    }
}
