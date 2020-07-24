using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Design;
using System.Drawing.Imaging;

namespace GUIDesigner
{
    class TableGridWidget : Panel, IWidget
    {
        public TableGridWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.Compress = true;
            this.External = false;
            this.Dither = true;
            this.LineColor = SystemColors.ActiveBorder;
            this.LineSize = 1;
            this.SingleSelect = false;
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Load, "", "");
        }

        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        public override Image BackgroundImage
        {
            get { return base.BackgroundImage; }
            set { base.BackgroundImage = value; }
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ImageLayout BackgroundImageLayout { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public BorderStyle BorderStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Cursor Cursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Font Font { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Color ForeColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual RightToLeft RightToLeft { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseWaitCursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AllowDrop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImeMode ImeMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int TabIndex { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool TabStop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string AccessibleDescription { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string AccessibleName { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public AccessibleRole AccessibleRole { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual AnchorStyles Anchor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AutoScroll { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Size AutoScrollMargin { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Size AutoScrollMinSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual DockStyle Dock { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Padding Margin { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Size MaximumSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Size MinimumSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Padding Padding { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool CausesValidation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ControlBindingsCollection DataBindings { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

        [LocalizedDescription("PixelFormat", typeof(WidgetManual))]
        public ITU.WidgetPixelFormat PixelFormat { get; set; }
        [LocalizedDescription("Compress", typeof(WidgetManual))]
        public Boolean Compress { get; set; }
        [LocalizedDescription("External", typeof(WidgetManual))]
        public Boolean External { get; set; }
        public Boolean Dither { get; set; }
        public bool SingleSelect { get; set; }
        public Color LineColor { get; set; }
        public int LineSize { get; set; }

        public class StringListConverter : TypeConverter
        {
            // Convert from a string.
            public override object ConvertFrom(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
            {
                if (value.GetType() == typeof(string))
                {
                    return value;
                }
                else
                {
                    return base.ConvertFrom(context, culture, value);
                }
            }

            public override bool
            GetStandardValuesSupported(ITypeDescriptorContext context)
            {
                return true; // display drop
            }
            public override bool
            GetStandardValuesExclusive(ITypeDescriptorContext context)
            {
                return true; // drop-down vs combo
            }
            public override StandardValuesCollection
            GetStandardValues(ITypeDescriptorContext context)
            {
                // note you can also look at context etc to build list
                return new StandardValuesCollection(NameCreationService.names.ToArray());
            }
        }

        public enum WidgetEvent
        {
            Timer = 0,
            MouseUp = 7,
            SlideUp = 11,
            SlideDown = 13,
            Load = 14,
            Select = 18,
            MouseLongPress = 21,
            Sync = 22,
            Delay0 = 25,
            Delay1 = 26,
            Delay2 = 27,
            Delay3 = 28,
            Delay4 = 29,
            Delay5 = 30,
            Delay6 = 31,
            Delay7 = 32,
            Custom0 = 100,
            Custom1 = 101,
            Custom2 = 102,
            Custom3 = 103,
            Custom4 = 104,
            Custom5 = 105,
            Custom6 = 106,
            Custom7 = 107,
            Custom8 = 108,
            Custom9 = 109,
            Custom10 = 110,
            Custom11 = 111,
            Custom12 = 112,
            Custom13 = 113,
            Custom14 = 114,
            Custom15 = 115,
            Custom16 = 116,
            Custom17 = 117,
            Custom18 = 118,
            Custom19 = 119,
            Custom20 = 120,
            Custom21 = 121,
            Custom22 = 122,
            Custom23 = 123,
            Custom24 = 124,
            Custom25 = 125,
            Custom26 = 126,
            Custom27 = 127,
            Custom28 = 128,
            Custom29 = 129,
            Custom30 = 130
        };

        public class WidgetActionTypeConverter : TypeConverter
        {
            public override bool CanConvertFrom(ITypeDescriptorContext context, Type sourceType)
            {
                if (sourceType == typeof(string)) return true;
                return base.CanConvertFrom(context, sourceType);
            }

            // Return true if we need to convert into a string.
            public override bool CanConvertTo(ITypeDescriptorContext context, Type destinationType)
            {
                if (destinationType == typeof(String)) return true;
                return base.CanConvertTo(context, destinationType);
            }

            // Convert from a string.
            public override object ConvertFrom(ITypeDescriptorContext context, CultureInfo culture, object value)
            {
                if (value.GetType() == typeof(string))
                {
                    // Split the string separated by commas.
                    string txt = (string)(value);
                    string[] fields = txt.Split(new char[] { ',' });

                    try
                    {
                        ITU.WidgetActionType a = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), fields[0]);
                        WidgetEvent e = WidgetEvent.Load;
                        try
                        {
                            e = (WidgetEvent)Enum.Parse(typeof(WidgetEvent), fields[1]);
                        }
                        catch
                        {
                            a = ITU.WidgetActionType.None;
                        }
                        return new WidgetAction(a, e, fields[2], fields[3]);
                    }
                    catch
                    {
                        throw new InvalidCastException(
                            "Cannot convert the string '" +
                            value.ToString() + "' into a Action");
                    }
                }
                else
                {
                    return base.ConvertFrom(context, culture, value);
                }
            }

            public override object ConvertTo(ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value, Type destinationType)
            {
                if (destinationType == typeof(string)) return value.ToString();
                return base.ConvertTo(context, culture, value, destinationType);
            }

            public override bool GetPropertiesSupported(ITypeDescriptorContext context)
            {
                return true;
            }

            public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
            {
                return TypeDescriptor.GetProperties(value);
            }
        }

        public class WidgetStringListConverter : TypeConverter
        {
            public override bool
            GetStandardValuesSupported(ITypeDescriptorContext context)
            {
                return true; // display drop
            }
            //public override bool
            //GetStandardValuesExclusive(ITypeDescriptorContext context)
            //{
            //    return true; // drop-down vs combo
            //}
            public override StandardValuesCollection
            GetStandardValues(ITypeDescriptorContext context)
            {
                List<string> names = new List<string>();
                WidgetAction a = (WidgetAction)context.Instance;

                foreach (HostSurface hs in HostSurface.hostSurfaces)
                {
                    Uitlity.GetWidgetNamesByActionType(hs.formDesignerHost.Container.Components, a.Action, names);
                }
                names.Sort();
                return new StandardValuesCollection(names.ToArray());
            }

            public override bool CanConvertFrom(System.ComponentModel.ITypeDescriptorContext context, System.Type sourceType)
            {
                if (sourceType == typeof(string))
                    return true;
                else
                    return base.CanConvertFrom(context, sourceType);
            }

            public override object ConvertFrom(System.ComponentModel.ITypeDescriptorContext context, System.Globalization.CultureInfo culture, object value)
            {
                if (value.GetType() == typeof(string))
                {
                    string s = value as string;
                    //NameCreationService.AddName(s);
                    return s;
                }
                else
                    return base.ConvertFrom(context, culture, value);
            }
        }

        [Serializable]
        [TypeConverter(typeof(WidgetActionTypeConverter))]
        public struct WidgetAction
        {
            public ITU.WidgetActionType Action { get; set; }
            public WidgetEvent Event { get; set; }

            private volatile string target;
            [TypeConverter(typeof(WidgetStringListConverter))]
            public String Target
            {
                get
                {
                    return target;
                }

                set
                {
                    if (Action == ITU.WidgetActionType.Function && ITU.enterKeyPressed && ITU.layerWidget != null && ITU.projectPath != null)
                    {
                        string funcName = value.Trim();

                        CodeGenerator.InvokeVisualStudio(funcName);

                        target = funcName;
                    }
                    else
                    {
                        target = value;
                    }
                }
            }

            public String Parameter { get; set; }

            public override string ToString()
            {
                return Action + "," + Event + "," + Target + "," + Parameter;
            }

            public WidgetAction(ITU.WidgetActionType action, WidgetEvent ev, String target, String param)
                : this()
            {
                Event = ev;
                Action = action;
                Target = target;
                Parameter = param;
            }
        }

        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action01 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action02 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action03 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action04 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action05 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action06 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action07 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action08 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action09 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action10 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action11 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action12 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action13 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action14 { get; set; }
        [LocalizedDescription("Action01to15", typeof(WidgetManual))]
        public WidgetAction Action15 { get; set; }

        private bool hided = false;
        [LocalizedDescription("Hided", typeof(WidgetManual))]
        public bool Hided
        {
            get
            {
                return hided;
            }

            set
            {
                if (value)
                    Hide();
                else
                    Show();

                hided = value;
            }
        }

        public ITUWidget CreateITUWidget()
        {
            ITUTableGrid grid = new ITUTableGrid();

            grid.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            grid.visible = (bool)properties["Visible"].GetValue(this);

            grid.active = false;
            grid.dirty = false;
            grid.alpha = 255;
            grid.rect.x = this.Location.X;
            grid.rect.y = this.Location.Y;
            grid.rect.width = this.Size.Width;
            grid.rect.height = this.Size.Height;
            grid.color.alpha = this.BackColor.A;
            grid.color.red = this.BackColor.R;
            grid.color.green = this.BackColor.G;
            grid.color.blue = this.BackColor.B;
            grid.bound.x = 0;
            grid.bound.y = 0;
            grid.bound.width = 0;
            grid.bound.height = 0;
            grid.lineColor.alpha = this.LineColor.A;
            grid.lineColor.red = this.LineColor.R;
            grid.lineColor.green = this.LineColor.G;
            grid.lineColor.blue = this.LineColor.B;
            grid.lineSize = this.LineSize;

            if (this.BackgroundImage != null)
            {
                grid.staticSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }
            if (this.External)
                grid.flags |= ITU.ITU_EXTERNAL;

            if (this.SingleSelect)
                grid.tableGridFlags |= ITUTableGrid.ITU_TABLEGRID_SINGLE_SELECT;

            grid.actions[0].action = (ITUActionType)this.Action01.Action;
            grid.actions[0].ev = (ITUEvent)this.Action01.Event;
            grid.actions[0].target = this.Action01.Target;
            grid.actions[0].param = this.Action01.Parameter;
            grid.actions[1].action = (ITUActionType)this.Action02.Action;
            grid.actions[1].ev = (ITUEvent)this.Action02.Event;
            grid.actions[1].target = this.Action02.Target;
            grid.actions[1].param = this.Action02.Parameter;
            grid.actions[2].action = (ITUActionType)this.Action03.Action;
            grid.actions[2].ev = (ITUEvent)this.Action03.Event;
            grid.actions[2].target = this.Action03.Target;
            grid.actions[2].param = this.Action03.Parameter;
            grid.actions[3].action = (ITUActionType)this.Action04.Action;
            grid.actions[3].ev = (ITUEvent)this.Action04.Event;
            grid.actions[3].target = this.Action04.Target;
            grid.actions[3].param = this.Action04.Parameter;
            grid.actions[4].action = (ITUActionType)this.Action05.Action;
            grid.actions[4].ev = (ITUEvent)this.Action05.Event;
            grid.actions[4].target = this.Action05.Target;
            grid.actions[4].param = this.Action05.Parameter;
            grid.actions[5].action = (ITUActionType)this.Action06.Action;
            grid.actions[5].ev = (ITUEvent)this.Action06.Event;
            grid.actions[5].target = this.Action06.Target;
            grid.actions[5].param = this.Action06.Parameter;
            grid.actions[6].action = (ITUActionType)this.Action07.Action;
            grid.actions[6].ev = (ITUEvent)this.Action07.Event;
            grid.actions[6].target = this.Action07.Target;
            grid.actions[6].param = this.Action07.Parameter;
            grid.actions[7].action = (ITUActionType)this.Action08.Action;
            grid.actions[7].ev = (ITUEvent)this.Action08.Event;
            grid.actions[7].target = this.Action08.Target;
            grid.actions[7].param = this.Action08.Parameter;
            grid.actions[8].action = (ITUActionType)this.Action09.Action;
            grid.actions[8].ev = (ITUEvent)this.Action09.Event;
            grid.actions[8].target = this.Action09.Target;
            grid.actions[8].param = this.Action09.Parameter;
            grid.actions[9].action = (ITUActionType)this.Action10.Action;
            grid.actions[9].ev = (ITUEvent)this.Action10.Event;
            grid.actions[9].target = this.Action10.Target;
            grid.actions[9].param = this.Action10.Parameter;
            grid.actions[10].action = (ITUActionType)this.Action11.Action;
            grid.actions[10].ev = (ITUEvent)this.Action11.Event;
            grid.actions[10].target = this.Action11.Target;
            grid.actions[10].param = this.Action11.Parameter;
            grid.actions[11].action = (ITUActionType)this.Action12.Action;
            grid.actions[11].ev = (ITUEvent)this.Action12.Event;
            grid.actions[11].target = this.Action12.Target;
            grid.actions[11].param = this.Action12.Parameter;
            grid.actions[12].action = (ITUActionType)this.Action13.Action;
            grid.actions[12].ev = (ITUEvent)this.Action13.Event;
            grid.actions[12].target = this.Action13.Target;
            grid.actions[12].param = this.Action13.Parameter;
            grid.actions[13].action = (ITUActionType)this.Action14.Action;
            grid.actions[13].ev = (ITUEvent)this.Action14.Event;
            grid.actions[13].target = this.Action14.Target;
            grid.actions[13].param = this.Action14.Parameter;
            grid.actions[14].action = (ITUActionType)this.Action15.Action;
            grid.actions[14].ev = (ITUEvent)this.Action15.Event;
            grid.actions[14].target = this.Action15.Target;
            grid.actions[14].param = this.Action15.Parameter;

            return grid;
        }

        public void SaveImages(String path)
        {
            if (this.BackgroundImage != null)
            {
                Bitmap bitmap = this.BackgroundImage as Bitmap;
                ITU.SaveImage(bitmap, path , LayerWidget.FindLayerName(this), this.Name + "_BackgroundImage");
            }
        }

        public void WriteFunctions(HashSet<string> functions)
        {
            if (this.Action01.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action01.Target);
            if (this.Action02.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action02.Target);
            if (this.Action03.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action03.Target);
            if (this.Action04.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action04.Target);
            if (this.Action05.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action05.Target);
            if (this.Action06.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action06.Target);
            if (this.Action07.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action07.Target);
            if (this.Action08.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action08.Target);
            if (this.Action09.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action09.Target);
            if (this.Action10.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action10.Target);
            if (this.Action11.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action11.Target);
            if (this.Action12.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action12.Target);
            if (this.Action13.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action13.Target);
            if (this.Action14.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action14.Target);
            if (this.Action15.Action == ITU.WidgetActionType.Function)
                functions.Add(this.Action15.Target);
        }

        public bool HasFunctionName(string funcName)
        {
            if (this.Action01.Action == ITU.WidgetActionType.Function && this.Action01.Target == funcName)
                return true;
            if (this.Action02.Action == ITU.WidgetActionType.Function && this.Action02.Target == funcName)
                return true;
            if (this.Action03.Action == ITU.WidgetActionType.Function && this.Action03.Target == funcName)
                return true;
            if (this.Action04.Action == ITU.WidgetActionType.Function && this.Action04.Target == funcName)
                return true;
            if (this.Action05.Action == ITU.WidgetActionType.Function && this.Action05.Target == funcName)
                return true;
            if (this.Action06.Action == ITU.WidgetActionType.Function && this.Action06.Target == funcName)
                return true;
            if (this.Action07.Action == ITU.WidgetActionType.Function && this.Action07.Target == funcName)
                return true;
            if (this.Action08.Action == ITU.WidgetActionType.Function && this.Action08.Target == funcName)
                return true;
            if (this.Action09.Action == ITU.WidgetActionType.Function && this.Action09.Target == funcName)
                return true;
            if (this.Action10.Action == ITU.WidgetActionType.Function && this.Action10.Target == funcName)
                return true;
            if (this.Action11.Action == ITU.WidgetActionType.Function && this.Action11.Target == funcName)
                return true;
            if (this.Action12.Action == ITU.WidgetActionType.Function && this.Action12.Target == funcName)
                return true;
            if (this.Action13.Action == ITU.WidgetActionType.Function && this.Action13.Target == funcName)
                return true;
            if (this.Action14.Action == ITU.WidgetActionType.Function && this.Action14.Target == funcName)
                return true;
            if (this.Action15.Action == ITU.WidgetActionType.Function && this.Action15.Target == funcName)
                return true;
            return false;
        }
    }
}
