using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;

namespace SmartPodVolumeWizard
{
    public class BoolToChineseConverter : IValueConverter
    {
        public object Convert(object value, Type targetType, object parameter, CultureInfo culture)
        {
            if (value is bool boolValue)
            {
                if (parameter is string displayTexts)
                {
                    int separatorIndex = displayTexts.IndexOf('|');
                    if (separatorIndex != -1 && separatorIndex != displayTexts.Length - 1)
                    {
                        var trueText = displayTexts.Substring(0, separatorIndex);
                        var falseText = displayTexts.Substring(separatorIndex + 1);
                        return boolValue ? trueText : falseText;
                    }
                }
                return boolValue ? "是" : "否";
            }
            return "";
        }

        public object ConvertBack(object value, Type targetType, object parameter, CultureInfo culture)
        {
            throw new NotImplementedException();
        }
    }
}
