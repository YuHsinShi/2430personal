using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Security.Cryptography;
using System.Text;
using System.Windows.Forms;
using System.Xml;
using SharpFont;

namespace GUIDesigner
{
    public class ITUCustom1 : ITUWidget
    {
        public ITUColor bgColor = new ITUColor();
        public int fontHeight;
        public int fontWidth;
        public int _string;
        public StringSetNode stringSet;

        public ITUCustom1()
        {
            this.type = ITUWidgetType.ITU_CUSTOM + 1;
        }

        public override int GetSize()
        {
            int size = base.GetSize();
            size += bgColor.GetSize();
            size += sizeof(int) * 4; // fontHeight, fontWidth, _string, stringSet pointer
            return size;
        }

        public override void Write(BinaryWriter bw, WidgetNode thisNode, WidgetNode parent, WidgetNode sibling, WidgetNode child)
        {
            base.Write(bw, thisNode, parent, sibling, child);
            this.bgColor.Write(bw);
            bw.Write(ITU.ToInt32(this.fontHeight));
            bw.Write(ITU.ToInt32(this.fontWidth));
            bw.Write(ITU.ToInt32(this._string));

            if (this.stringSet != null)
                bw.Write(ITU.ToUInt32(this.stringSet.offset));
            else
                bw.Write((uint)0);
        }    
    }
}
