using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Documents;

namespace SmartPodVolumeWizard
{
    internal class ConfKey
    {
        public const string Blacklist = "blacklist"; // object
        public const string Whitelist = "whitelist"; // object

        public const string FriendlyName = "friendly_name"; // string
        public const string ExpectedVolume = "expected_vol"; // float
        public const string Description = "description"; // string
        public const string Mute = "mute"; // bool
    }
}
