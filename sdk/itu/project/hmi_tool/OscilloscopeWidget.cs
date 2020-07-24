using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class OscilloscopeWidget : Panel, IWidget
    {
        public OscilloscopeWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.ResolutionX = 50;
            this.ResolutionY = 10;
            this.LineWidth = 1;
            this.GridResolutionX = 10;
            this.GridResolutionY = 10;
            this.GridLineWidth = 1;
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Image BackgroundImage { get; set; }

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

        public int ResolutionX { get; set; }
        public int ResolutionY { get; set; }
        public int LineWidth { get; set; }

        public int GridResolutionX { get; set; }
        public int GridResolutionY { get; set; }
        public int GridLineWidth { get; set; }
        public Color GridColor { get; set; }

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
            ITUOscilloscope osc = new ITUOscilloscope();

            osc.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            osc.visible = (bool)properties["Visible"].GetValue(this);

            osc.active = false;
            osc.dirty = false;
            osc.alpha = 255;
            osc.rect.x = this.Location.X;
            osc.rect.y = this.Location.Y;
            osc.rect.width = this.Size.Width;
            osc.rect.height = this.Size.Height;
            osc.color.alpha = this.BackColor.A;
            osc.color.red = this.BackColor.R;
            osc.color.green = this.BackColor.G;
            osc.color.blue = this.BackColor.B;
            osc.bound.x = 0;
            osc.bound.y = 0;
            osc.bound.width = 0;
            osc.bound.height = 0;

            osc.resolutionX = this.ResolutionX;
            osc.resolutionY = this.ResolutionY;
            osc.lineWidth = this.LineWidth;
            osc.fgColor.alpha = this.ForeColor.A;
            osc.fgColor.red = this.ForeColor.R;
            osc.fgColor.green = this.ForeColor.G;
            osc.fgColor.blue = this.ForeColor.B;

            osc.gridResolutionX = this.GridResolutionX;
            osc.gridResolutionY = this.GridResolutionY;
            osc.gridLineWidth = this.GridLineWidth;
            osc.gridColor.alpha = this.GridColor.A;
            osc.gridColor.red = this.GridColor.R;
            osc.gridColor.green = this.GridColor.G;
            osc.gridColor.blue = this.GridColor.B;

            return osc;
        }

        public void SaveImages(String path)
        {
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
