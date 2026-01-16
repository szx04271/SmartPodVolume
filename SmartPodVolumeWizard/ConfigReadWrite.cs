using Newtonsoft.Json;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Automation.Peers;
using System.Windows.Media.Media3D;

namespace SmartPodVolumeWizard
{
    using DeviceMap = Dictionary<string, ConfigReadWrite.DeviceInfo>;

    public class ConfigReadWrite
    {
        public class DeviceInfo
        {
            [JsonProperty(ConfKey.FriendlyName)]
            public string FriendlyName { get; set; }

            [JsonProperty(ConfKey.Description)]
            public string Description { get; set; }

            [JsonProperty(ConfKey.Mute, NullValueHandling = NullValueHandling.Ignore)]
            public bool? Mute { get; set; }

            [JsonProperty(ConfKey.ExpectedVolume, NullValueHandling = NullValueHandling.Ignore)]
            public float? VolumePercent { get; set; }
        }

        public const string ConfigFileName = "config.json";

        public static JObject TryGetConfigJson()
        {
            try
            {
                var configText = File.ReadAllText(ConfigFileName, Encoding.UTF8);
                return JObject.Parse(configText); // if the root item in the file isn't an object, this throws exception
            }
            catch { }
            return null;
        }

        public static DeviceMap GetWhiteListDevices()
        {
            DeviceMap ret = new DeviceMap();

            var configJson = TryGetConfigJson();
            if (configJson == null)
            {
                return ret;
            }

            try
            {
                var whiteListJson = (JObject)configJson[ConfKey.Whitelist];
                // VS调试器默认在NullReferenceException引发时中断调试（即使之后可能被catch），
                // 可见此种异常的严重性。因此我们最好避免。
                if (whiteListJson == null)
                {
                    return ret;
                }
                foreach (var item in whiteListJson)
                {
                    try
                    {
                        var deviceJson = (JObject)item.Value;
                        DeviceInfo deviceInfo = new DeviceInfo
                        {
                            VolumePercent = (float?)deviceJson[ConfKey.ExpectedVolume],
                            Mute = (bool?)deviceJson[ConfKey.Mute],
                            FriendlyName = (string)deviceJson[ConfKey.FriendlyName],
                            Description = (string)deviceJson[ConfKey.Description]
                        };
                        ret.Add(item.Key, deviceInfo);
                    }
                    catch { }
                }
            }
            catch { }

            return ret;
        }

        public static DeviceMap GetBlackListDevices()
        {
            DeviceMap ret = new DeviceMap();

            var configJson = TryGetConfigJson();
            if (configJson == null)
            {
                return ret;
            }

            try
            {
                var blackListJson = (JObject)configJson[ConfKey.Blacklist];
                if (blackListJson == null)
                {
                    return ret;
                }
                foreach (var item in blackListJson)
                {
                    try
                    {
                        var deviceJson = (JObject)item.Value;
                        DeviceInfo deviceInfo = new DeviceInfo
                        {
                            FriendlyName = (string)deviceJson[ConfKey.FriendlyName],
                            Description = (string)deviceJson[ConfKey.Description]
                        };
                        ret.Add(item.Key, deviceInfo);
                    }
                    catch { }
                }
            }
            catch { }

            return ret;
        }

        public static void SaveConfig(DeviceMap white, DeviceMap black)
        {
            var whiteListJson = JObject.FromObject(white);
            var blackListJson = JObject.FromObject(black);
            JObject configJson = new JObject
            {
                {ConfKey.Whitelist, whiteListJson},
                {ConfKey.Blacklist, blackListJson}
            };
            try
            {
                File.WriteAllText(ConfigFileName, configJson.ToString(), Encoding.UTF8);
            }
            catch { }
        }
    }
}
