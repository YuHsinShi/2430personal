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
    class CheckBoxWidget : CheckBox, IWidget
    {
        public CheckBoxWidget()
        {
            base.Appearance = System.Windows.Forms.Appearance.Button;
            base.BackColor = SystemColors.ButtonFace;
            base.BackgroundImageLayout = ImageLayout.None;
            base.FlatAppearance.CheckedBackColor = SystemColors.InactiveCaption;
            base.FlatAppearance.BorderColor = SystemColors.WindowFrame;
            base.FlatAppearance.MouseDownBackColor = Color.Transparent;
            base.FlatAppearance.MouseOverBackColor = Color.Transparent;
            this.CheckedFontColor = this.ForeColor;
            base.FlatStyle = System.Windows.Forms.FlatStyle.Flat;
            base.Margin = new Padding(0, 0, 0, 0);
            this.SupportTouch = true;
            this.TransformType = IconWidget.WidgetTransformType.None;
            this.TransformX = 100;
            this.TransformY = 100;
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Press, "", "");
            this.Compress = true;
            this.External = false;
            this.Dither = true;
            this.BoldSize = 1;
            this.Font = new Font(this.Font.Name, this.Font.Size, GraphicsUnit.Pixel);
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Appearance Appearance { get; set; }

        [Browsable(false)]
        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        public override Image BackgroundImage
        {
            get { return base.BackgroundImage; }
            set
            {
                if (previewImage == CheckBoxImageType.Default && this.DefaultImage == null)
                    base.BackgroundImage = this.DefaultImage = value;
            }
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ImageLayout BackgroundImageLayout { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ContentAlignment CheckAlign { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public CheckState CheckState { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Cursor Cursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public FlatStyle FlatStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public Image Image { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ContentAlignment ImageAlign { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int ImageIndex { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public string ImageKey { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImageList ImageList { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual RightToLeft RightToLeft { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public TextImageRelation TextImageRelation { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseMnemonic { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseVisualStyleBackColor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseWaitCursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AllowDrop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AutoCheck { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AutoEllipsis { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ThreeState { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseCompatibleTextRendering { get; set; }

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
        public override bool AutoSize { get; set; }

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
        [LocalizedDescription("ResponseToUpKey", typeof(WidgetManual))]
        public Boolean ResponseToUpKey { get; set; }

        [LocalizedDescription("CheckedFontColor", typeof(WidgetManual))]
        public Color CheckedFontColor { get; set; }

        private Image focusImage;
        [LocalizedDescription("FocusImage", typeof(WidgetManual))]
        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusImage
        {
            get { return focusImage; }
            set
            {
                this.focusImage = value;
                if (previewImage == CheckBoxImageType.Focus)
                    base.BackgroundImage = value;
            }
        }

        private Image pressImage;
        [LocalizedDescription("PressImage", typeof(WidgetManual))]
        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image PressImage
        {
            get { return pressImage; }
            set
            {
                this.pressImage = value;
            }
        }

        private Image checkedImage;
        [LocalizedDescription("CheckedImage", typeof(WidgetManual))]
        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image CheckedImage
        {
            get { return checkedImage; }
            set
            {
                this.checkedImage = value;
                if (previewImage == CheckBoxImageType.Checked)
                    base.BackgroundImage = value;
            }
        }

        private Image focusCheckedImage;
        [LocalizedDescription("FocusCheckedImage", typeof(WidgetManual))]
        [Editor(typeof(ImageLocationEditor), typeof(UITypeEditor))]
        [DefaultValue(typeof(Image), null)]
        public Image FocusCheckedImage
        {
            get { return focusCheckedImage; }
            set
            {
                this.focusCheckedImage = value;
                if (previewImage == CheckBoxImageType.FocusChecked)
                    base.BackgroundImage = value;
            }
        }

        private Image defaultImage;
        public Image DefaultImage
        {
            get { return defaultImage; }
            set
            {
                this.defaultImage = value;
                if (previewImage == CheckBoxImageType.Default)
                    base.BackgroundImage = value;
            }
        }

        public enum CheckBoxImageType
        {
            Default,
            Focus,
            Checked,
            FocusChecked
        };

        private CheckBoxImageType previewImage = CheckBoxImageType.Default;
        public CheckBoxImageType PreviewImage
        {
            get { return previewImage; }
            set
            {
                switch (value)
                {
                    case CheckBoxImageType.Focus:
                        base.BackgroundImage = this.FocusImage;
                        break;

                    case CheckBoxImageType.Checked:
                        base.BackgroundImage = this.CheckedImage;
                        break;

                    case CheckBoxImageType.FocusChecked:
                        base.BackgroundImage = this.FocusCheckedImage;
                        break;

                    default:
                        base.BackgroundImage = this.DefaultImage;
                        break;
                }
                previewImage = value;
            }
        }

        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text1 { get; set; }
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text2 { get; set; }
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text3 { get; set; }
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text4 { get; set; }
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text5 { get; set; }
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text6 { get; set; }
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text7 { get; set; }
        public string Text8 { get; set; }
        public string Text9 { get; set; }
        public string Text10 { get; set; }
        public string Text11 { get; set; }
        public string Text12 { get; set; }
        public string Text13 { get; set; }
        public string Text14 { get; set; }
        public string Text15 { get; set; }
        public string Text16 { get; set; }
        public string Text17 { get; set; }
        public string Text18 { get; set; }
        public string Text19 { get; set; }
        public string Text20 { get; set; }
        public string Text21 { get; set; }
        public string Text22 { get; set; }
        public string Text23 { get; set; }
        public string Text24 { get; set; }
        public string Text25 { get; set; }
        public string Text26 { get; set; }
        public string Text27 { get; set; }
        public string Text28 { get; set; }
        public string Text29 { get; set; }
        public string Text30 { get; set; }
        public string Text31 { get; set; }
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

        [LocalizedDescription("Stretch", typeof(WidgetManual))]
        public Boolean Stretch
        {
            get { return base.BackgroundImageLayout == ImageLayout.Stretch; }
            set { base.BackgroundImageLayout = value ? ImageLayout.Stretch : ImageLayout.None; }
        }

        [LocalizedDescription("SupportTouch", typeof(WidgetManual))]
        public Boolean SupportTouch { get; set; }

        [LocalizedDescription("Angle", typeof(WidgetManual))]
        public int Angle { get; set; }

        [LocalizedDescription("TransformType", typeof(WidgetManual))]
        public IconWidget.WidgetTransformType TransformType { get; set; }

        private int transformX = 0;
        [LocalizedDescription("TransformX", typeof(WidgetManual))]
        public int TransformX
        {
            get
            {
                return transformX;
            }

            set
            {
                if (value < 0)
                    transformX = 0;
                else if (value > ITU.ITU_TRANSFORM_MAX_VALUE)
                    transformX = ITU.ITU_TRANSFORM_MAX_VALUE;
                else
                    transformX = value;
            }
        }

        private int transformY = 0;
        [LocalizedDescription("TransformY", typeof(WidgetManual))]
        public int TransformY
        {
            get
            {
                return transformY;
            }

            set
            {
                if (value < 0)
                    transformY = 0;
                else if (value > ITU.ITU_TRANSFORM_MAX_VALUE)
                    transformY = ITU.ITU_TRANSFORM_MAX_VALUE;
                else
                    transformY = value;
            }
        }

        public enum WidgetEvent
        {
            Timer = 0,
            Press = 1,
            KeyDown = 4,
            KeyUp = 5,
            MouseDown = 6,
            MouseUp = 7,
            MouseDoubleClick = 8,
            SlideLeft = 10,
            SlideUp = 11,
            SlideRight = 12,
            SlideDown = 13,
            MouseLongPress = 21,
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
                        WidgetEvent e = WidgetEvent.Press;
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
            ITUCheckBox checkbox = new ITUCheckBox();

            checkbox.name = this.Name;
            checkbox.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;
            checkbox.flags |= this.ResponseToUpKey ? ITU.ITU_RESPONSE_TO_UP_KEY : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            checkbox.visible = (bool)properties["Visible"].GetValue(this);

            checkbox.active = false;
            checkbox.dirty = false;
            checkbox.alpha = 255;
            checkbox.tabIndex = this.TabIndex;
            checkbox.rect.x = this.Location.X;
            checkbox.rect.y = this.Location.Y;
            checkbox.rect.width = this.Size.Width;
            checkbox.rect.height = this.Size.Height;
            checkbox.color.alpha = this.BackColor.A;
            checkbox.color.red = this.BackColor.R;
            checkbox.color.green = this.BackColor.G;
            checkbox.color.blue = this.BackColor.B;
            checkbox.bound.x = 0;
            checkbox.bound.y = 0;
            checkbox.bound.width = 0;
            checkbox.bound.height = 0;

            if (this.DefaultImage != null)
            {
                checkbox.staticSurf = ITU.CreateSurfaceNode(this.DefaultImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }
            if (this.External)
                checkbox.flags |= ITU.ITU_EXTERNAL;

            ITUBorderWindow bwin = checkbox.bwin.widget as ITUBorderWindow;
            bwin.name = "";
            bwin.visible = true;
            bwin.active = false;
            bwin.dirty = false;
            bwin.alpha = 255;
            bwin.rect.x = 0;
            bwin.rect.y = 0;
            bwin.rect.width = this.Size.Width;
            bwin.rect.height = this.Size.Height;
            bwin.color.alpha = this.FlatAppearance.BorderColor.A;
            bwin.color.red = this.FlatAppearance.BorderColor.R;
            bwin.color.green = this.FlatAppearance.BorderColor.G;
            bwin.color.blue = this.FlatAppearance.BorderColor.B;
            bwin.bound.x = 0;
            bwin.bound.y = 0;
            bwin.bound.width = 0;
            bwin.bound.height = 0;
            bwin.borderSize = this.FlatAppearance.BorderSize;

            ITUText text = checkbox.text.widget as ITUText;
            text.flags = ITU.ITU_CLIP_DISABLED;
            text.name = "";
            text.visible = true;
            text.active = false;
            text.dirty = false;
            text.alpha = 255;
            text.rect.x = 0;
            text.rect.y = 0;

            using (Graphics cg = this.CreateGraphics())
            {
                SizeF size = cg.MeasureString(this.Text, this.Font);

                text.rect.width = (int)Math.Ceiling(size.Width);
                text.rect.height = (int)Math.Ceiling(size.Height);
            }
            text.color.alpha = this.ForeColor.A;
            text.color.red = this.ForeColor.R;
            text.color.green = this.ForeColor.G;
            text.color.blue = this.ForeColor.B;
            text.bound.x = 0;
            text.bound.y = 0;
            text.bound.width = 0;
            text.bound.height = 0;
            text.fontWidth = (int)Math.Round(this.Font.Size);
            text.fontHeight = text.fontWidth;
            text.fontIndex = this.FontIndex;
            text.layout = ITULayout.ITU_LAYOUT_MIDDLE_CENTER;

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

            string[] texts = new string[] { this.Text, this.Text1, this.Text2, this.Text3, this.Text4, this.Text5, this.Text6, this.Text7, this.Text8, this.Text9, this.Text10,
                this.Text11, this.Text12, this.Text13, this.Text14, this.Text15, this.Text16, this.Text17, this.Text18, this.Text19, this.Text20,
                this.Text21, this.Text22, this.Text23, this.Text24, this.Text25, this.Text26, this.Text27, this.Text28, this.Text29, this.Text30,
                this.Text31 };
            text.stringSet = ITU.CreateStringSetNode(texts);

            checkbox.bgColor.alpha = this.BackColor.A;
            checkbox.bgColor.red = this.BackColor.R;
            checkbox.bgColor.green = this.BackColor.G;
            checkbox.bgColor.blue = this.BackColor.B;
            checkbox.focusColor.alpha = this.FlatAppearance.MouseOverBackColor.A;
            checkbox.focusColor.red = this.FlatAppearance.MouseOverBackColor.R;
            checkbox.focusColor.green = this.FlatAppearance.MouseOverBackColor.G;
            checkbox.focusColor.blue = this.FlatAppearance.MouseOverBackColor.B;

            if (this.Stretch)
                checkbox.flags |= ITU.ITU_STRETCH;

            if (this.FocusImage != null)
            {
                checkbox.staticFocusSurf = ITU.CreateSurfaceNode(this.FocusImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }

            if (this.PressImage != null)
            {
                checkbox.staticPressSurf = ITU.CreateSurfaceNode(this.PressImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }
            checkbox.pressColor.alpha = this.FlatAppearance.MouseDownBackColor.A;
            checkbox.pressColor.red = this.FlatAppearance.MouseDownBackColor.R;
            checkbox.pressColor.green = this.FlatAppearance.MouseDownBackColor.G;
            checkbox.pressColor.blue = this.FlatAppearance.MouseDownBackColor.B;
            checkbox.pressed = 0;

            checkbox.checkedColor.alpha = this.FlatAppearance.CheckedBackColor.A;
            checkbox.checkedColor.red = this.FlatAppearance.CheckedBackColor.R;
            checkbox.checkedColor.green = this.FlatAppearance.CheckedBackColor.G;
            checkbox.checkedColor.blue = this.FlatAppearance.CheckedBackColor.B;

            checkbox.checkedFontColor.alpha = this.CheckedFontColor.A;
            checkbox.checkedFontColor.red = this.CheckedFontColor.R;
            checkbox.checkedFontColor.green = this.CheckedFontColor.G;
            checkbox.checkedFontColor.blue = this.CheckedFontColor.B;
            checkbox.orgFontColor.alpha = this.ForeColor.A;
            checkbox.orgFontColor.red = this.ForeColor.R;
            checkbox.orgFontColor.green = this.ForeColor.G;
            checkbox.orgFontColor.blue = this.ForeColor.B;

            if (this.CheckedImage != null)
            {
                checkbox.staticCheckedSurf = ITU.CreateSurfaceNode(this.CheckedImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }
            checkbox._checked = this.Checked ? 1 : 0;

            if (this.FocusCheckedImage != null)
            {
                checkbox.staticFocusCheckedSurf = ITU.CreateSurfaceNode(this.FocusCheckedImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }

            if (!this.SupportTouch)
                checkbox.flags &= ~ITU.ITU_ENABLED;

            checkbox.angle = this.Angle;
            checkbox.transformType = (ITUTransformType)this.TransformType;
            checkbox.transformX = this.TransformX;
            checkbox.transformY = this.TransformY;

            checkbox.actions[0].action = (ITUActionType)this.Action01.Action;
            checkbox.actions[0].ev = (ITUEvent)this.Action01.Event;
            checkbox.actions[0].target = this.Action01.Target;
            checkbox.actions[0].param = this.Action01.Parameter;
            checkbox.actions[1].action = (ITUActionType)this.Action02.Action;
            checkbox.actions[1].ev = (ITUEvent)this.Action02.Event;
            checkbox.actions[1].target = this.Action02.Target;
            checkbox.actions[1].param = this.Action02.Parameter;
            checkbox.actions[2].action = (ITUActionType)this.Action03.Action;
            checkbox.actions[2].ev = (ITUEvent)this.Action03.Event;
            checkbox.actions[2].target = this.Action03.Target;
            checkbox.actions[2].param = this.Action03.Parameter;
            checkbox.actions[3].action = (ITUActionType)this.Action04.Action;
            checkbox.actions[3].ev = (ITUEvent)this.Action04.Event;
            checkbox.actions[3].target = this.Action04.Target;
            checkbox.actions[3].param = this.Action04.Parameter;
            checkbox.actions[4].action = (ITUActionType)this.Action05.Action;
            checkbox.actions[4].ev = (ITUEvent)this.Action05.Event;
            checkbox.actions[4].target = this.Action05.Target;
            checkbox.actions[4].param = this.Action05.Parameter;
            checkbox.actions[5].action = (ITUActionType)this.Action06.Action;
            checkbox.actions[5].ev = (ITUEvent)this.Action06.Event;
            checkbox.actions[5].target = this.Action06.Target;
            checkbox.actions[5].param = this.Action06.Parameter;
            checkbox.actions[6].action = (ITUActionType)this.Action07.Action;
            checkbox.actions[6].ev = (ITUEvent)this.Action07.Event;
            checkbox.actions[6].target = this.Action07.Target;
            checkbox.actions[6].param = this.Action07.Parameter;
            checkbox.actions[7].action = (ITUActionType)this.Action08.Action;
            checkbox.actions[7].ev = (ITUEvent)this.Action08.Event;
            checkbox.actions[7].target = this.Action08.Target;
            checkbox.actions[7].param = this.Action08.Parameter;
            checkbox.actions[8].action = (ITUActionType)this.Action09.Action;
            checkbox.actions[8].ev = (ITUEvent)this.Action09.Event;
            checkbox.actions[8].target = this.Action09.Target;
            checkbox.actions[8].param = this.Action09.Parameter;
            checkbox.actions[9].action = (ITUActionType)this.Action10.Action;
            checkbox.actions[9].ev = (ITUEvent)this.Action10.Event;
            checkbox.actions[9].target = this.Action10.Target;
            checkbox.actions[9].param = this.Action10.Parameter;
            checkbox.actions[10].action = (ITUActionType)this.Action11.Action;
            checkbox.actions[10].ev = (ITUEvent)this.Action11.Event;
            checkbox.actions[10].target = this.Action11.Target;
            checkbox.actions[10].param = this.Action11.Parameter;
            checkbox.actions[11].action = (ITUActionType)this.Action12.Action;
            checkbox.actions[11].ev = (ITUEvent)this.Action12.Event;
            checkbox.actions[11].target = this.Action12.Target;
            checkbox.actions[11].param = this.Action12.Parameter;
            checkbox.actions[12].action = (ITUActionType)this.Action13.Action;
            checkbox.actions[12].ev = (ITUEvent)this.Action13.Event;
            checkbox.actions[12].target = this.Action13.Target;
            checkbox.actions[12].param = this.Action13.Parameter;
            checkbox.actions[13].action = (ITUActionType)this.Action14.Action;
            checkbox.actions[13].ev = (ITUEvent)this.Action14.Event;
            checkbox.actions[13].target = this.Action14.Target;
            checkbox.actions[13].param = this.Action14.Parameter;
            checkbox.actions[14].action = (ITUActionType)this.Action15.Action;
            checkbox.actions[14].ev = (ITUEvent)this.Action15.Event;
            checkbox.actions[14].target = this.Action15.Target;
            checkbox.actions[14].param = this.Action15.Parameter;

            return checkbox;
        }

        public void SaveImages(String path)
        {
            if (this.BackgroundImage != null)
            {
                Bitmap bitmap = this.BackgroundImage as Bitmap;
                ITU.SaveImage(bitmap, path , LayerWidget.FindLayerName(this), this.Name + "_BackgroundImage");
            }

            if (this.FocusImage != null)
            {
                Bitmap bitmap = this.FocusImage as Bitmap;
                ITU.SaveImage(bitmap, path , LayerWidget.FindLayerName(this), this.Name + "_FocusImage");
            }

            if (this.PressImage != null)
            {
                Bitmap bitmap = this.PressImage as Bitmap;
                ITU.SaveImage(bitmap, path, LayerWidget.FindLayerName(this), this.Name + "_PressImage");
            }

            if (this.CheckedImage != null)
            {
                Bitmap bitmap = this.CheckedImage as Bitmap;
                ITU.SaveImage(bitmap, path , LayerWidget.FindLayerName(this), this.Name + "_CheckedImage");
            }

            if (this.FocusCheckedImage != null)
            {
                Bitmap bitmap = this.FocusCheckedImage as Bitmap;
                ITU.SaveImage(bitmap, path , LayerWidget.FindLayerName(this), this.Name + "_FocusCheckedImage");
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
