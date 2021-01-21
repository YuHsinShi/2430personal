using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using SharpFont;

namespace GUIDesigner
{
    class TextWidget : Label, IWidget
    {
        public TextWidget()
        {
            base.Margin = new Padding(0, 0, 0, 0);
            this.BoldSize = 1;
            this.Font = new Font(this.Font.Name, this.Font.Size, GraphicsUnit.Pixel);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (ITU.fontFace != null)
            {
                uint fontWidth = (uint)Math.Round(this.Font.Size);
                int boldSize = base.Font.Bold ? this.BoldSize : 0;
                ITU.DrawText(fontWidth, this.ClientSize, this.Text, this.TextAlign, boldSize, this.ForeColor, e.Graphics);
            }
            else
                base.OnPaint(e);
        }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual BorderStyle BorderStyle { get; set; }

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
        new public bool UseMnemonic { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseWaitCursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AllowDrop { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AutoEllipsis { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public int TabIndex { get; set; }

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
        new public virtual DockStyle Dock { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

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
        new public ControlBindingsCollection DataBindings { get; set;  }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

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

        public int LetterSpacing { get; set; }

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
            ITUText widget = new ITUText();

            widget.name = this.Name;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            widget.visible = (bool)properties["Visible"].GetValue(this);

            widget.active = false;
            widget.dirty = false;
            widget.alpha = 255;
            widget.rect.x = this.Location.X;
            widget.rect.y = this.Location.Y;
            widget.rect.width = this.Size.Width;
            widget.rect.height = this.Size.Height;
            widget.color.alpha = this.ForeColor.A;
            widget.color.red = this.ForeColor.R;
            widget.color.green = this.ForeColor.G;
            widget.color.blue = this.ForeColor.B;
            widget.bound.x = 0;
            widget.bound.y = 0;
            widget.bound.width = 0;
            widget.bound.height = 0;
            widget.bgColor.alpha = this.BackColor.A;
            widget.bgColor.red = this.BackColor.R;
            widget.bgColor.green = this.BackColor.G;
            widget.bgColor.blue = this.BackColor.B;
            widget.fontWidth = (int)Math.Round(this.Font.Size);
            widget.fontHeight = widget.fontWidth;
            widget.fontIndex = this.FontIndex;

            if (base.Font.Bold)
                widget.textFlags |= ITUText.ITU_TEXT_BOLD;

            widget.boldSize = this.BoldSize;

            widget.arabicIndices = 0;
            widget.hebrewIndices = 0;

            if (this.IsArabic)
                widget.arabicIndices |= 1u << 0;

            if (this.IsArabic1)
                widget.arabicIndices |= 1u << 1;

            if (this.IsArabic2)
                widget.arabicIndices |= 1u << 2;

            if (this.IsArabic3)
                widget.arabicIndices |= 1u << 3;

            if (this.IsArabic4)
                widget.arabicIndices |= 1u << 4;

            if (this.IsArabic5)
                widget.arabicIndices |= 1u << 5;

            if (this.IsArabic6)
                widget.arabicIndices |= 1u << 6;

            if (this.IsArabic7)
                widget.arabicIndices |= 1u << 7;

            if (this.IsArabic8)
                widget.arabicIndices |= 1u << 8;

            if (this.IsArabic9)
                widget.arabicIndices |= 1u << 9;

            if (this.IsArabic10)
                widget.arabicIndices |= 1u << 10;

            if (this.IsArabic11)
                widget.arabicIndices |= 1u << 11;

            if (this.IsArabic12)
                widget.arabicIndices |= 1u << 12;

            if (this.IsArabic13)
                widget.arabicIndices |= 1u << 13;

            if (this.IsArabic14)
                widget.arabicIndices |= 1u << 14;

            if (this.IsArabic15)
                widget.arabicIndices |= 1u << 15;

            if (this.IsArabic16)
                widget.arabicIndices |= 1u << 16;

            if (this.IsArabic17)
                widget.arabicIndices |= 1u << 17;

            if (this.IsArabic18)
                widget.arabicIndices |= 1u << 18;

            if (this.IsArabic19)
                widget.arabicIndices |= 1u << 19;

            if (this.IsArabic20)
                widget.arabicIndices |= 1u << 20;

            if (this.IsArabic21)
                widget.arabicIndices |= 1u << 21;

            if (this.IsArabic22)
                widget.arabicIndices |= 1u << 22;

            if (this.IsArabic23)
                widget.arabicIndices |= 1u << 23;

            if (this.IsArabic24)
                widget.arabicIndices |= 1u << 24;

            if (this.IsArabic25)
                widget.arabicIndices |= 1u << 25;

            if (this.IsArabic26)
                widget.arabicIndices |= 1u << 26;

            if (this.IsArabic27)
                widget.arabicIndices |= 1u << 27;

            if (this.IsArabic28)
                widget.arabicIndices |= 1u << 28;

            if (this.IsArabic29)
                widget.arabicIndices |= 1u << 29;

            if (this.IsArabic30)
                widget.arabicIndices |= 1u << 30;

            if (this.IsArabic31)
                widget.arabicIndices |= 1u << 31;

            if (widget.arabicIndices != 0)
                widget.textFlags |= ITUText.ITU_TEXT_ARABIC;

            if (this.IsHebrew)
                widget.hebrewIndices |= 1u << 0;

            if (this.IsHebrew1)
                widget.hebrewIndices |= 1u << 1;

            if (this.IsHebrew2)
                widget.hebrewIndices |= 1u << 2;

            if (this.IsHebrew3)
                widget.hebrewIndices |= 1u << 3;

            if (this.IsHebrew4)
                widget.hebrewIndices |= 1u << 4;

            if (this.IsHebrew5)
                widget.hebrewIndices |= 1u << 5;

            if (this.IsHebrew6)
                widget.hebrewIndices |= 1u << 6;

            if (this.IsHebrew7)
                widget.hebrewIndices |= 1u << 7;

            if (this.IsHebrew8)
                widget.hebrewIndices |= 1u << 8;

            if (this.IsHebrew9)
                widget.hebrewIndices |= 1u << 9;

            if (this.IsHebrew10)
                widget.hebrewIndices |= 1u << 10;

            if (this.IsHebrew11)
                widget.hebrewIndices |= 1u << 11;

            if (this.IsHebrew12)
                widget.hebrewIndices |= 1u << 12;

            if (this.IsHebrew13)
                widget.hebrewIndices |= 1u << 13;

            if (this.IsHebrew14)
                widget.hebrewIndices |= 1u << 14;

            if (this.IsHebrew15)
                widget.hebrewIndices |= 1u << 15;

            if (this.IsHebrew16)
                widget.hebrewIndices |= 1u << 16;

            if (this.IsHebrew17)
                widget.hebrewIndices |= 1u << 17;

            if (this.IsHebrew18)
                widget.hebrewIndices |= 1u << 18;

            if (this.IsHebrew19)
                widget.hebrewIndices |= 1u << 19;

            if (this.IsHebrew20)
                widget.hebrewIndices |= 1u << 20;

            if (this.IsHebrew21)
                widget.hebrewIndices |= 1u << 21;

            if (this.IsHebrew22)
                widget.hebrewIndices |= 1u << 22;

            if (this.IsHebrew23)
                widget.hebrewIndices |= 1u << 23;

            if (this.IsHebrew24)
                widget.hebrewIndices |= 1u << 24;

            if (this.IsHebrew25)
                widget.hebrewIndices |= 1u << 25;

            if (this.IsHebrew26)
                widget.hebrewIndices |= 1u << 26;

            if (this.IsHebrew27)
                widget.hebrewIndices |= 1u << 27;

            if (this.IsHebrew28)
                widget.hebrewIndices |= 1u << 28;

            if (this.IsHebrew29)
                widget.hebrewIndices |= 1u << 29;

            if (this.IsHebrew30)
                widget.hebrewIndices |= 1u << 30;

            if (this.IsHebrew31)
                widget.hebrewIndices |= 1u << 31;

            if (widget.hebrewIndices != 0)
                widget.textFlags |= ITUText.ITU_TEXT_HEBREW;

            widget.letterSpacing = this.LetterSpacing;

            switch (this.TextAlign)
            {
            case ContentAlignment.BottomLeft:
                widget.layout = ITULayout.ITU_LAYOUT_BOTTOM_LEFT;
                break;

            case ContentAlignment.MiddleLeft:
                widget.layout = ITULayout.ITU_LAYOUT_MIDDLE_LEFT;
                break;
            
            case ContentAlignment.TopLeft:
                widget.layout = ITULayout.ITU_LAYOUT_TOP_LEFT;
                break;

            case ContentAlignment.BottomCenter:
                widget.layout = ITULayout.ITU_LAYOUT_BOTTOM_CENTER;
                break;

            case ContentAlignment.MiddleCenter:
                widget.layout = ITULayout.ITU_LAYOUT_MIDDLE_CENTER;
                break;

            case ContentAlignment.TopCenter:
                widget.layout = ITULayout.ITU_LAYOUT_TOP_CENTER;
                break;

            case ContentAlignment.BottomRight:
                widget.layout = ITULayout.ITU_LAYOUT_BOTTOM_RIGHT;
                break;

            case ContentAlignment.MiddleRight:
                widget.layout = ITULayout.ITU_LAYOUT_MIDDLE_RIGHT;
                break;

            case ContentAlignment.TopRight:
                widget.layout = ITULayout.ITU_LAYOUT_TOP_RIGHT;
                break; 
           
            default:
                widget.layout = ITULayout.ITU_LAYOUT_DEFAULT;
                break;
            }

            string[] texts = new string[] { this.Text, this.Text1, this.Text2, this.Text3, this.Text4, this.Text5, this.Text6, this.Text7, this.Text8, this.Text9, this.Text10,
                this.Text11, this.Text12, this.Text13, this.Text14, this.Text15, this.Text16, this.Text17, this.Text18, this.Text19, this.Text20,
                this.Text21, this.Text22, this.Text23, this.Text24, this.Text25, this.Text26, this.Text27, this.Text28, this.Text29, this.Text30,
                this.Text31 };
            widget.stringSet = ITU.CreateStringSetNode(texts);

            return widget;
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
