using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Drawing.Design;
using System.Drawing.Imaging;

namespace GUIDesigner
{
    class AnalogClockWidget : Panel, IWidget
    {
        public AnalogClockWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.Compress = true;
            this.External = false;
            this.Dither = true;
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

        [LocalizedDescription("HourTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String HourTarget { get; set; }

        [LocalizedDescription("HourX", typeof(WidgetManual))]
        public int HourX { get; set; }
        [LocalizedDescription("HourY", typeof(WidgetManual))]
        public int HourY { get; set; }

        [LocalizedDescription("MinuteTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String MinuteTarget { get; set; }

        [LocalizedDescription("MinuteX", typeof(WidgetManual))]
        public int MinuteX { get; set; }
        [LocalizedDescription("MinuteY", typeof(WidgetManual))]
        public int MinuteY { get; set; }

        [LocalizedDescription("SecondTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String SecondTarget { get; set; }

        [LocalizedDescription("SecondX", typeof(WidgetManual))]
        public int SecondX { get; set; }
        [LocalizedDescription("SecondY", typeof(WidgetManual))]
        public int SecondY { get; set; }

        [LocalizedDescription("Stretch", typeof(WidgetManual))]
        public Boolean Stretch
        {
            get { return base.BackgroundImageLayout == ImageLayout.Stretch; }
            set { base.BackgroundImageLayout = value ? ImageLayout.Stretch : ImageLayout.None; }
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
            ITUAnalogClock aclk = new ITUAnalogClock();

            aclk.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            aclk.visible = (bool)properties["Visible"].GetValue(this);

            aclk.active = false;
            aclk.dirty = false;
            aclk.alpha = 255;
            aclk.rect.x = this.Location.X;
            aclk.rect.y = this.Location.Y;
            aclk.rect.width = this.Size.Width;
            aclk.rect.height = this.Size.Height;
            aclk.color.alpha = this.BackColor.A;
            aclk.color.red = this.BackColor.R;
            aclk.color.green = this.BackColor.G;
            aclk.color.blue = this.BackColor.B;
            aclk.bound.x = 0;
            aclk.bound.y = 0;
            aclk.bound.width = 0;
            aclk.bound.height = 0;

            if (this.Stretch)
                aclk.flags |= ITU.ITU_STRETCH;

            if (this.BackgroundImage != null)
            {
                aclk.staticSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }
            if (this.External)
                aclk.flags |= ITU.ITU_EXTERNAL;

            aclk.hourName = this.HourTarget;
            aclk.hourX = this.HourX;
            aclk.hourY = this.HourY;
            aclk.minuteName = this.MinuteTarget;
            aclk.minuteX = this.MinuteX;
            aclk.minuteY = this.MinuteY;
            aclk.secondName = this.SecondTarget;
            aclk.secondX = this.SecondX;
            aclk.secondY = this.SecondY;

            return aclk;
        }

        public void SaveImages(String path)
        {
            if (this.BackgroundImage != null)
            {
                Bitmap bitmap = this.BackgroundImage as Bitmap;
                ITU.SaveImage(bitmap, path, LayerWidget.FindLayerName(this), this.Name + "_BackgroundImage");
            }
        }

        public void WriteFunctions(HashSet<string> functions)
        {
        }

        public bool HasFunctionName(string funcName)
        {
            return false;
        }
    }
}
