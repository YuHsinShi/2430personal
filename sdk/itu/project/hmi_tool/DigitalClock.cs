﻿using System;
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
    class DigitalClockWidget : Panel, IWidget
    {
        public DigitalClockWidget()
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
        public override bool AutoSize { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual AutoSizeMode AutoSizeMode { get; set; }

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
        [LocalizedDescription("AMPM", typeof(WidgetManual))]
        public bool AMPM { get; set; }

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

        [LocalizedDescription("YearTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String YearTarget { get; set; }

        [LocalizedDescription("MonthTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String MonthTarget { get; set; }

        [LocalizedDescription("DigitalClockDayTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String DayTarget { get; set; }

        [LocalizedDescription("DigitalClockHourTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String HourTarget { get; set; }

        [LocalizedDescription("DigitalClockMinuteTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String MinuteTarget { get; set; }

        [LocalizedDescription("DigitalClockSecondTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String SecondTarget { get; set; }

        [LocalizedDescription("WeekTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String WeekTarget { get; set; }

        [LocalizedDescription("ColonTarget", typeof(WidgetManual))]
        [TypeConverter(typeof(StringListConverter))]
        public String ColonTarget { get; set; }

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
            ITUDigitalClock dclk = new ITUDigitalClock();

            dclk.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            dclk.visible = (bool)properties["Visible"].GetValue(this);

            dclk.active = false;
            dclk.dirty = false;
            dclk.alpha = 255;
            dclk.rect.x = this.Location.X;
            dclk.rect.y = this.Location.Y;
            dclk.rect.width = this.Size.Width;
            dclk.rect.height = this.Size.Height;
            dclk.color.alpha = this.BackColor.A;
            dclk.color.red = this.BackColor.R;
            dclk.color.green = this.BackColor.G;
            dclk.color.blue = this.BackColor.B;
            dclk.bound.x = 0;
            dclk.bound.y = 0;
            dclk.bound.width = 0;
            dclk.bound.height = 0;

            if (this.BackgroundImage != null)
            {
                dclk.staticSurf = ITU.CreateSurfaceNode(this.BackgroundImage as Bitmap, this.PixelFormat, this.Compress, this.External, this.Dither);
            }
            if (this.External)
                dclk.flags |= ITU.ITU_EXTERNAL;

            dclk.digitalClockFlags = this.AMPM ? ITUDigitalClock.ITU_DIGITALCLOCK_12H : 0;
            dclk.yearName = this.YearTarget;
            dclk.monthName = this.MonthTarget;
            dclk.dayName = this.DayTarget;
            dclk.hourName = this.HourTarget;
            dclk.minuteName = this.MinuteTarget;
            dclk.secondName = this.SecondTarget;
            dclk.weekName = this.WeekTarget;
            dclk.colonName = this.ColonTarget;

            return dclk;
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
