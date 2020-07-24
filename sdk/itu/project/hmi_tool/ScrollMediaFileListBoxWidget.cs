using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class ScrollMediaFileListBoxWidget : ListBox, IWidget
    {
        public ScrollMediaFileListBoxWidget()
        {
            base.BorderStyle = System.Windows.Forms.BorderStyle.None;
            base.Margin = new Padding(0, 0, 0, 0);
            this.FocusColor = SystemColors.ActiveCaption;
            this.FocusFontColor = this.ForeColor;
            this.ReadFontColor = SystemColors.GrayText;
            this.BackAlpha = 255;
            this.TextAlign = ContentAlignment.TopLeft;
            this.Path = "A:/";
            this.Extensions = "mp3;wma";
            this.ScrollDelay = ITU.ITU_SCROLL_DELAY;
            this.StopDelay = ITU.ITU_STOP_DELAY;
            this.ItemHeight = base.ItemHeight;
            this.TotalFrame = 10;
            this.Draggable = true;
            this.BoldSize = 1;
            this.Font = new Font(this.Font.Name, this.Font.Size, GraphicsUnit.Pixel);
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

            this.FontChanged += new EventHandler(ScrollMediaFileListBoxWidget_FontChanged);
        }

        void ScrollMediaFileListBoxWidget_FontChanged(object sender, EventArgs e)
        {
            if (this.ItemHeight < base.ItemHeight)
                this.ItemHeight = base.ItemHeight;
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public BorderStyle BorderStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Cursor Cursor { get; set; }

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
        new public int ColumnWidth { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual DrawMode DrawMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int HorizontalExtent { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]        
        new public bool HorizontalScrollbar { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImeMode ImeMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool IntegralHeight { get; set; }

        new public int ItemHeight { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool MultiColumn { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ScrollAlwaysVisible { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual SelectionMode SelectionMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Sorted { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseTabStops { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string FormatString { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool FormattingEnabled { get; set; }

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
        new public bool CausesValidation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ControlBindingsCollection DataBindings { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object DataSource { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string DisplayMember { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string ValueMember { get; set; }

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

        [LocalizedDescription("TextAlign", typeof(WidgetManual))]
        public ContentAlignment TextAlign { get; set; }

        [LocalizedDescription("PageIndexTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]        
        public string PageIndexTarget { get; set; }

        [LocalizedDescription("PageCountTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]        
        public string PageCountTarget { get; set; }

        [LocalizedDescription("FocusColor", typeof(WidgetManual))]
        public Color FocusColor { get; set; }
        [LocalizedDescription("FocusFontColor", typeof(WidgetManual))]
        public Color FocusFontColor { get; set; }
        [LocalizedDescription("ReadFontColor", typeof(WidgetManual))]
        public Color ReadFontColor { get; set; }
        [LocalizedDescription("BackAlpha", typeof(WidgetManual))]
        public byte BackAlpha { get; set; }
        [LocalizedDescription("Path", typeof(WidgetManual))]
        public string Path { get; set; }
        [LocalizedDescription("Extensions", typeof(WidgetManual))]
        public string Extensions { get; set; }
        [LocalizedDescription("ScrollDelay", typeof(WidgetManual))]
        public int ScrollDelay { get; set; }
        [LocalizedDescription("StopDelay", typeof(WidgetManual))]
        public int StopDelay { get; set; }
        [LocalizedDescription("TotalFrame", typeof(WidgetManual))]
        public int TotalFrame { get; set; }
        [LocalizedDescription("Draggable", typeof(WidgetManual))]
        public bool Draggable { get; set; }
        [LocalizedDescription("FontIndex", typeof(WidgetManual))]
        public int FontIndex { get; set; }
        [LocalizedDescription("BoldSize", typeof(WidgetManual))]
        public int BoldSize { get; set; }
        public bool IsArabic { get; set; }
        public bool IsArabic1 { get; set; }
        public bool IsArabic2 { get; set; }
        public bool IsArabic3 { get; set; }
        public bool IsArabic4 { get; set; }
        public bool IsArabic5 { get; set; }
        public bool IsArabic6 { get; set; }
        public bool IsArabic7 { get; set; }
        public bool IsArabic8 { get; set; }
        public bool IsArabic9 { get; set; }
        public bool IsArabic10 { get; set; }
        public bool IsArabic11 { get; set; }
        public bool IsArabic12 { get; set; }
        public bool IsArabic13 { get; set; }
        public bool IsArabic14 { get; set; }
        public bool IsArabic15 { get; set; }
        public bool IsArabic16 { get; set; }
        public bool IsArabic17 { get; set; }
        public bool IsArabic18 { get; set; }
        public bool IsArabic19 { get; set; }
        public bool IsArabic20 { get; set; }
        public bool IsArabic21 { get; set; }
        public bool IsArabic22 { get; set; }
        public bool IsArabic23 { get; set; }
        public bool IsArabic24 { get; set; }
        public bool IsArabic25 { get; set; }
        public bool IsArabic26 { get; set; }
        public bool IsArabic27 { get; set; }
        public bool IsArabic28 { get; set; }
        public bool IsArabic29 { get; set; }
        public bool IsArabic30 { get; set; }
        public bool IsArabic31 { get; set; }
        public bool IsHebrew { get; set; }
        public bool IsHebrew1 { get; set; }
        public bool IsHebrew2 { get; set; }
        public bool IsHebrew3 { get; set; }
        public bool IsHebrew4 { get; set; }
        public bool IsHebrew5 { get; set; }
        public bool IsHebrew6 { get; set; }
        public bool IsHebrew7 { get; set; }
        public bool IsHebrew8 { get; set; }
        public bool IsHebrew9 { get; set; }
        public bool IsHebrew10 { get; set; }
        public bool IsHebrew11 { get; set; }
        public bool IsHebrew12 { get; set; }
        public bool IsHebrew13 { get; set; }
        public bool IsHebrew14 { get; set; }
        public bool IsHebrew15 { get; set; }
        public bool IsHebrew16 { get; set; }
        public bool IsHebrew17 { get; set; }
        public bool IsHebrew18 { get; set; }
        public bool IsHebrew19 { get; set; }
        public bool IsHebrew20 { get; set; }
        public bool IsHebrew21 { get; set; }
        public bool IsHebrew22 { get; set; }
        public bool IsHebrew23 { get; set; }
        public bool IsHebrew24 { get; set; }
        public bool IsHebrew25 { get; set; }
        public bool IsHebrew26 { get; set; }
        public bool IsHebrew27 { get; set; }
        public bool IsHebrew28 { get; set; }
        public bool IsHebrew29 { get; set; }
        public bool IsHebrew30 { get; set; }
        public bool IsHebrew31 { get; set; }

        public enum WidgetEvent
        {
            Timer = 0,
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
        
        protected override CreateParams CreateParams
        {
            get
            {
                CreateParams cp = base.CreateParams;
                cp.Style &= ~0x200000;  // Turn off WS_VSCROLL
                return cp;
            }
        }

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
            ITUScrollMediaFileListBox smflistbox = new ITUScrollMediaFileListBox();

            smflistbox.type = ITUWidgetType.ITU_SCROLLMEDIAFILELISTBOX;
            smflistbox.name = this.Name;
            smflistbox.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            smflistbox.visible = (bool)properties["Visible"].GetValue(this);

            smflistbox.active = false;
            smflistbox.dirty = false;
            smflistbox.alpha = 255;
            smflistbox.tabIndex = this.TabIndex;
            smflistbox.rect.x = this.Location.X;
            smflistbox.rect.y = this.Location.Y;
            smflistbox.rect.width = this.Size.Width;
            smflistbox.rect.height = this.Size.Height;
            smflistbox.color.alpha = this.BackAlpha;
            smflistbox.color.red = this.BackColor.R;
            smflistbox.color.green = this.BackColor.G;
            smflistbox.color.blue = this.BackColor.B;
            smflistbox.bound.x = 0;
            smflistbox.bound.y = 0;
            smflistbox.bound.width = 0;
            smflistbox.bound.height = 0;

            for (int i = 0; i < 3; i++)
            {
                foreach (object item in this.Items)
                {
                    ITUScrollText text = new ITUScrollText();

                    text.name = "";
                    text.visible = true;
                    text.active = false;
                    text.dirty = false;
                    text.alpha = 255;
                    text.rect.x = 0;
                    text.rect.y = 0;
                    text.rect.width = this.Size.Width;
                    text.rect.height = this.ItemHeight;
                    text.color.alpha = this.ForeColor.A;
                    text.color.red = this.ForeColor.R;
                    text.color.green = this.ForeColor.G;
                    text.color.blue = this.ForeColor.B;
                    text.bound.x = 0;
                    text.bound.y = 0;
                    text.bound.width = 0;
                    text.bound.height = 0;
                    text.bgColor.alpha = 0;
                    text.bgColor.red = 0;
                    text.bgColor.green = 0;
                    text.bgColor.blue = 0;
                    text.fontWidth = (int)Math.Round(this.Font.Size);
                    text.fontHeight = text.fontWidth;
                    text.fontIndex = this.FontIndex;
                    text.width = this.Size.Width;

                    if (base.Font.Bold)
                        text.textFlags |= ITUText.ITU_TEXT_BOLD;

                    text.boldSize = this.BoldSize;

                    text.arabicIndices = 0;
                    text.hebrewIndices = 0;

                    if (this.IsArabic)
                        text.arabicIndices |= 1u << 0;

                    if (this.IsArabic1)
                        text.arabicIndices |= 1u << 1;

                    if (this.IsArabic2)
                        text.arabicIndices |= 1u << 2;

                    if (this.IsArabic3)
                        text.arabicIndices |= 1u << 3;

                    if (this.IsArabic4)
                        text.arabicIndices |= 1u << 4;

                    if (this.IsArabic5)
                        text.arabicIndices |= 1u << 5;

                    if (this.IsArabic6)
                        text.arabicIndices |= 1u << 6;

                    if (this.IsArabic7)
                        text.arabicIndices |= 1u << 7;

                    if (this.IsArabic8)
                        text.arabicIndices |= 1u << 8;

                    if (this.IsArabic9)
                        text.arabicIndices |= 1u << 9;

                    if (this.IsArabic10)
                        text.arabicIndices |= 1u << 10;

                    if (this.IsArabic11)
                        text.arabicIndices |= 1u << 11;

                    if (this.IsArabic12)
                        text.arabicIndices |= 1u << 12;

                    if (this.IsArabic13)
                        text.arabicIndices |= 1u << 13;

                    if (this.IsArabic14)
                        text.arabicIndices |= 1u << 14;

                    if (this.IsArabic15)
                        text.arabicIndices |= 1u << 15;

                    if (this.IsArabic16)
                        text.arabicIndices |= 1u << 16;

                    if (this.IsArabic17)
                        text.arabicIndices |= 1u << 17;

                    if (this.IsArabic18)
                        text.arabicIndices |= 1u << 18;

                    if (this.IsArabic19)
                        text.arabicIndices |= 1u << 19;

                    if (this.IsArabic20)
                        text.arabicIndices |= 1u << 20;

                    if (this.IsArabic21)
                        text.arabicIndices |= 1u << 21;

                    if (this.IsArabic22)
                        text.arabicIndices |= 1u << 22;

                    if (this.IsArabic23)
                        text.arabicIndices |= 1u << 23;

                    if (this.IsArabic24)
                        text.arabicIndices |= 1u << 24;

                    if (this.IsArabic25)
                        text.arabicIndices |= 1u << 25;

                    if (this.IsArabic26)
                        text.arabicIndices |= 1u << 26;

                    if (this.IsArabic27)
                        text.arabicIndices |= 1u << 27;

                    if (this.IsArabic28)
                        text.arabicIndices |= 1u << 28;

                    if (this.IsArabic29)
                        text.arabicIndices |= 1u << 29;

                    if (this.IsArabic30)
                        text.arabicIndices |= 1u << 30;

                    if (this.IsArabic31)
                        text.arabicIndices |= 1u << 31;

                    if (text.arabicIndices != 0)
                        text.textFlags |= ITUText.ITU_TEXT_ARABIC;

                    if (this.IsHebrew)
                        text.hebrewIndices |= 1u << 0;

                    if (this.IsHebrew1)
                        text.hebrewIndices |= 1u << 1;

                    if (this.IsHebrew2)
                        text.hebrewIndices |= 1u << 2;

                    if (this.IsHebrew3)
                        text.hebrewIndices |= 1u << 3;

                    if (this.IsHebrew4)
                        text.hebrewIndices |= 1u << 4;

                    if (this.IsHebrew5)
                        text.hebrewIndices |= 1u << 5;

                    if (this.IsHebrew6)
                        text.hebrewIndices |= 1u << 6;

                    if (this.IsHebrew7)
                        text.hebrewIndices |= 1u << 7;

                    if (this.IsHebrew8)
                        text.hebrewIndices |= 1u << 8;

                    if (this.IsHebrew9)
                        text.hebrewIndices |= 1u << 9;

                    if (this.IsHebrew10)
                        text.hebrewIndices |= 1u << 10;

                    if (this.IsHebrew11)
                        text.hebrewIndices |= 1u << 11;

                    if (this.IsHebrew12)
                        text.hebrewIndices |= 1u << 12;

                    if (this.IsHebrew13)
                        text.hebrewIndices |= 1u << 13;

                    if (this.IsHebrew14)
                        text.hebrewIndices |= 1u << 14;

                    if (this.IsHebrew15)
                        text.hebrewIndices |= 1u << 15;

                    if (this.IsHebrew16)
                        text.hebrewIndices |= 1u << 16;

                    if (this.IsHebrew17)
                        text.hebrewIndices |= 1u << 17;

                    if (this.IsHebrew18)
                        text.hebrewIndices |= 1u << 18;

                    if (this.IsHebrew19)
                        text.hebrewIndices |= 1u << 19;

                    if (this.IsHebrew20)
                        text.hebrewIndices |= 1u << 20;

                    if (this.IsHebrew21)
                        text.hebrewIndices |= 1u << 21;

                    if (this.IsHebrew22)
                        text.hebrewIndices |= 1u << 22;

                    if (this.IsHebrew23)
                        text.hebrewIndices |= 1u << 23;

                    if (this.IsHebrew24)
                        text.hebrewIndices |= 1u << 24;

                    if (this.IsHebrew25)
                        text.hebrewIndices |= 1u << 25;

                    if (this.IsHebrew26)
                        text.hebrewIndices |= 1u << 26;

                    if (this.IsHebrew27)
                        text.hebrewIndices |= 1u << 27;

                    if (this.IsHebrew28)
                        text.hebrewIndices |= 1u << 28;

                    if (this.IsHebrew29)
                        text.hebrewIndices |= 1u << 29;

                    if (this.IsHebrew30)
                        text.hebrewIndices |= 1u << 30;

                    if (this.IsHebrew31)
                        text.hebrewIndices |= 1u << 31;

                    if (text.hebrewIndices != 0)
                        text.textFlags |= ITUText.ITU_TEXT_HEBREW;

                    switch (this.TextAlign)
                    {
                        case ContentAlignment.BottomLeft:
                            text.layout = ITULayout.ITU_LAYOUT_BOTTOM_LEFT;
                            break;

                        case ContentAlignment.MiddleLeft:
                            text.layout = ITULayout.ITU_LAYOUT_MIDDLE_LEFT;
                            break;

                        case ContentAlignment.TopLeft:
                            text.layout = ITULayout.ITU_LAYOUT_TOP_LEFT;
                            break;

                        case ContentAlignment.BottomCenter:
                            text.layout = ITULayout.ITU_LAYOUT_BOTTOM_CENTER;
                            break;

                        case ContentAlignment.MiddleCenter:
                            text.layout = ITULayout.ITU_LAYOUT_MIDDLE_CENTER;
                            break;

                        case ContentAlignment.TopCenter:
                            text.layout = ITULayout.ITU_LAYOUT_TOP_CENTER;
                            break;

                        case ContentAlignment.BottomRight:
                            text.layout = ITULayout.ITU_LAYOUT_BOTTOM_RIGHT;
                            break;

                        case ContentAlignment.MiddleRight:
                            text.layout = ITULayout.ITU_LAYOUT_MIDDLE_RIGHT;
                            break;

                        case ContentAlignment.TopRight:
                            text.layout = ITULayout.ITU_LAYOUT_TOP_RIGHT;
                            break;

                        default:
                            text.layout = ITULayout.ITU_LAYOUT_DEFAULT;
                            break;
                    }

                    text.scrollDelay = this.ScrollDelay;
                    text.stopDelay = this.StopDelay;
                    text.scrollTextState = 1;

                    WidgetNode node = new WidgetNode();
                    node.widget = text;
                    smflistbox.items.Add(node);
                }
            }
            smflistbox.pageIndexName       = this.PageIndexTarget;
            smflistbox.pageCountName       = this.PageCountTarget;
            smflistbox.focusColor.alpha    = this.FocusColor.A;
            smflistbox.focusColor.red      = this.FocusColor.R;
            smflistbox.focusColor.green    = this.FocusColor.G;
            smflistbox.focusColor.blue     = this.FocusColor.B;
            smflistbox.focusFontColor.alpha = this.FocusFontColor.A;
            smflistbox.focusFontColor.red = this.FocusFontColor.R;
            smflistbox.focusFontColor.green = this.FocusFontColor.G;
            smflistbox.focusFontColor.blue = this.FocusFontColor.B;
            smflistbox.orgFontColor.alpha = this.ForeColor.A;
            smflistbox.orgFontColor.red = this.ForeColor.R;
            smflistbox.orgFontColor.green = this.ForeColor.G;
            smflistbox.orgFontColor.blue = this.ForeColor.B;
            smflistbox.readFontColor.alpha = this.ReadFontColor.A;
            smflistbox.readFontColor.red = this.ReadFontColor.R;
            smflistbox.readFontColor.green = this.ReadFontColor.G;
            smflistbox.readFontColor.blue = this.ReadFontColor.B;
            smflistbox.scrollDelay         = this.ScrollDelay;
            smflistbox.stopDelay           = this.StopDelay;
            smflistbox.path                = this.Path;
            smflistbox.extensions          = this.Extensions;
            smflistbox.totalframe          = this.TotalFrame;

            if (this.Draggable)
                smflistbox.flags |= ITU.ITU_DRAGGABLE;

            smflistbox.actions[0].action = (ITUActionType)this.Action01.Action;
            smflistbox.actions[0].ev = (ITUEvent)this.Action01.Event;
            smflistbox.actions[0].target = this.Action01.Target;
            smflistbox.actions[0].param = this.Action01.Parameter;
            smflistbox.actions[1].action = (ITUActionType)this.Action02.Action;
            smflistbox.actions[1].ev = (ITUEvent)this.Action02.Event;
            smflistbox.actions[1].target = this.Action02.Target;
            smflistbox.actions[1].param = this.Action02.Parameter;
            smflistbox.actions[2].action = (ITUActionType)this.Action03.Action;
            smflistbox.actions[2].ev = (ITUEvent)this.Action03.Event;
            smflistbox.actions[2].target = this.Action03.Target;
            smflistbox.actions[2].param = this.Action03.Parameter;
            smflistbox.actions[3].action = (ITUActionType)this.Action04.Action;
            smflistbox.actions[3].ev = (ITUEvent)this.Action04.Event;
            smflistbox.actions[3].target = this.Action04.Target;
            smflistbox.actions[3].param = this.Action04.Parameter;
            smflistbox.actions[4].action = (ITUActionType)this.Action05.Action;
            smflistbox.actions[4].ev = (ITUEvent)this.Action05.Event;
            smflistbox.actions[4].target = this.Action05.Target;
            smflistbox.actions[4].param = this.Action05.Parameter;
            smflistbox.actions[5].action = (ITUActionType)this.Action06.Action;
            smflistbox.actions[5].ev = (ITUEvent)this.Action06.Event;
            smflistbox.actions[5].target = this.Action06.Target;
            smflistbox.actions[5].param = this.Action06.Parameter;
            smflistbox.actions[6].action = (ITUActionType)this.Action07.Action;
            smflistbox.actions[6].ev = (ITUEvent)this.Action07.Event;
            smflistbox.actions[6].target = this.Action07.Target;
            smflistbox.actions[6].param = this.Action07.Parameter;
            smflistbox.actions[7].action = (ITUActionType)this.Action08.Action;
            smflistbox.actions[7].ev = (ITUEvent)this.Action08.Event;
            smflistbox.actions[7].target = this.Action08.Target;
            smflistbox.actions[7].param = this.Action08.Parameter;
            smflistbox.actions[8].action = (ITUActionType)this.Action09.Action;
            smflistbox.actions[8].ev = (ITUEvent)this.Action09.Event;
            smflistbox.actions[8].target = this.Action09.Target;
            smflistbox.actions[8].param = this.Action09.Parameter;
            smflistbox.actions[9].action = (ITUActionType)this.Action10.Action;
            smflistbox.actions[9].ev = (ITUEvent)this.Action10.Event;
            smflistbox.actions[9].target = this.Action10.Target;
            smflistbox.actions[9].param = this.Action10.Parameter;
            smflistbox.actions[10].action = (ITUActionType)this.Action11.Action;
            smflistbox.actions[10].ev = (ITUEvent)this.Action11.Event;
            smflistbox.actions[10].target = this.Action11.Target;
            smflistbox.actions[10].param = this.Action11.Parameter;
            smflistbox.actions[11].action = (ITUActionType)this.Action12.Action;
            smflistbox.actions[11].ev = (ITUEvent)this.Action12.Event;
            smflistbox.actions[11].target = this.Action12.Target;
            smflistbox.actions[11].param = this.Action12.Parameter;
            smflistbox.actions[12].action = (ITUActionType)this.Action13.Action;
            smflistbox.actions[12].ev = (ITUEvent)this.Action13.Event;
            smflistbox.actions[12].target = this.Action13.Target;
            smflistbox.actions[12].param = this.Action13.Parameter;
            smflistbox.actions[13].action = (ITUActionType)this.Action14.Action;
            smflistbox.actions[13].ev = (ITUEvent)this.Action14.Event;
            smflistbox.actions[13].target = this.Action14.Target;
            smflistbox.actions[13].param = this.Action14.Parameter;
            smflistbox.actions[14].action = (ITUActionType)this.Action15.Action;
            smflistbox.actions[14].ev = (ITUEvent)this.Action15.Event;
            smflistbox.actions[14].target = this.Action15.Target;
            smflistbox.actions[14].param = this.Action15.Parameter;

            return smflistbox;
        }

        public void SaveImages(String path)
        {
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
