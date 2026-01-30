using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;

namespace SmartPodVolumeWizard
{
    public class ListViewWidthRefresher
    {
        // ListView在列宽设为auto时只会在初次填入数据时自动计算宽度。
        // 此函数强制其再次计算。
        public static void RefreshWidths(ListView listView)
        {
            var gv = (GridView)listView.View;
            if (gv != null)
            {
                foreach (GridViewColumn column in gv.Columns)
                {
                    column.Width = column.ActualWidth;
                    column.Width = Double.NaN;
                }
            }
        }
    }
}
