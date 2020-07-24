using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Drawing;
using System.Drawing.Design;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    class StepWheelWidget : ListBox, IWidget
    {
        public StepWheelWidget()
        {
            base.BorderStyle = System.Windows.Forms.BorderStyle.None;
            base.Margin = new Padding(0, 0, 0, 0);
            this.FocusColor = SystemColors.ActiveCaption;
            this.FocusIndex = 1;
            //this.ItemCount = 3;
            this.Cycle = false;
            //this.Speed = 2;
            //this.SlideCount = 3;
            this.BackAlpha = 255;
            this.TextAlign = ContentAlignment.MiddleCenter;
            this.ItemHeight = base.ItemHeight;
            this.TotalFrame = 10;
            this.Draggable = true;
            this.Touchable = true;
            this.FontSquare = false;
            this.MouseUpChangeFactor = 2;
            this.DragChangeFactor = 2;
            this.Slidable = true;
            this.FocusFontHeight = (int)this.Font.SizeInPoints;
            this.StepFontHeight1 = (int)this.Font.SizeInPoints;
            this.StepFontHeight2 = (int)this.Font.SizeInPoints;
            this.BoldSize = 1;
            this.Font = new Font(this.Font.Name, this.Font.Size, GraphicsUnit.Pixel);
            this.Items1 = new ObjectCollection(new ListBox());
            this.Items2 = new ObjectCollection(new ListBox());
            this.Items3 = new ObjectCollection(new ListBox());
            this.Items4 = new ObjectCollection(new ListBox());
            this.Items5 = new ObjectCollection(new ListBox());
            this.Items6 = new ObjectCollection(new ListBox());
            this.Items7 = new ObjectCollection(new ListBox());
            this.Items8 = new ObjectCollection(new ListBox());
            this.Items9 = new ObjectCollection(new ListBox());
            this.Items10 = new ObjectCollection(new ListBox());
            this.Items11 = new ObjectCollection(new ListBox());
            this.Items12 = new ObjectCollection(new ListBox());
            this.Items13 = new ObjectCollection(new ListBox());
            this.Items14 = new ObjectCollection(new ListBox());
            this.Items15 = new ObjectCollection(new ListBox());
            this.Items16 = new ObjectCollection(new ListBox());
            this.Items17 = new ObjectCollection(new ListBox());
            this.Items18 = new ObjectCollection(new ListBox());
            this.Items19 = new ObjectCollection(new ListBox());
            this.Items20 = new ObjectCollection(new ListBox());
            this.Items21 = new ObjectCollection(new ListBox());
            this.Items22 = new ObjectCollection(new ListBox());
            this.Items23 = new ObjectCollection(new ListBox());
            this.Items24 = new ObjectCollection(new ListBox());
            this.Items25 = new ObjectCollection(new ListBox());
            this.Items26 = new ObjectCollection(new ListBox());
            this.Items27 = new ObjectCollection(new ListBox());
            this.Items28 = new ObjectCollection(new ListBox());
            this.Items29 = new ObjectCollection(new ListBox());
            this.Items30 = new ObjectCollection(new ListBox());
            this.Items31 = new ObjectCollection(new ListBox());
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.Changed, "", "");

            this.FontChanged += new EventHandler(StepWheelWidget_FontChanged);
        }

        void StepWheelWidget_FontChanged(object sender, EventArgs e)
        {
            if (this.ItemHeight < base.ItemHeight)
                this.ItemHeight = base.ItemHeight;

            if (this.FocusFontHeight < (int)this.Font.SizeInPoints)
                this.FocusFontHeight = (int)this.Font.SizeInPoints;
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

        [LocalizedDescription("TextAlign", typeof(WidgetManual))]
        public ContentAlignment TextAlign { get; set; }

        [LocalizedDescription("FocusColor", typeof(WidgetManual))]
        public Color FocusColor { get; set; }
        [LocalizedDescription("FocusIndex", typeof(WidgetManual))]
        public int FocusIndex { get; set; }
        [LocalizedDescription("TotalFrame", typeof(WidgetManual))]
        public int TotalFrame { get; set; }
        [LocalizedDescription("BackAlpha", typeof(WidgetManual))]
        public byte BackAlpha { get; set; }
        [LocalizedDescription("Draggable", typeof(WidgetManual))]
        public bool Draggable { get; set; }
        [LocalizedDescription("FocusFontHeight", typeof(WidgetManual))]
        public int FocusFontHeight { get; set; }
        [LocalizedDescription("StepFontHeight1", typeof(WidgetManual))]
        public int StepFontHeight1 { get; set; }
        [LocalizedDescription("StepFontHeight2", typeof(WidgetManual))]
        public int StepFontHeight2 { get; set; }
        [LocalizedDescription("FontIndex", typeof(WidgetManual))]
        public int FontIndex { get; set; }
        [LocalizedDescription("BoldSize", typeof(WidgetManual))]
        public int BoldSize { get; set; }
        [LocalizedDescription("Cycle", typeof(WidgetManual))]
        public bool Cycle { get; set; }
        [LocalizedDescription("FontSquare", typeof(WidgetManual))]
        public bool FontSquare{ get; set; }
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

        private int mouseup_cf = 2;
        [LocalizedDescription("MouseUpChangeFactor", typeof(WidgetManual))]
        public int MouseUpChangeFactor
        {
            get { return mouseup_cf; }
            set
            {
                if (value < 1)
                {
                    mouseup_cf = 1;
                }
                else if (value > 4)
                {
                    mouseup_cf = 4;
                }
                else
                {
                    mouseup_cf = value;
                }
            }
        }

        private int drag_cf = 2;
        [LocalizedDescription("DragChangeFactor", typeof(WidgetManual))]
        public int DragChangeFactor
        {
            get { return drag_cf; }
            set
            {
                if (value < 1)
                {
                    drag_cf = 1;
                }
                else if (value > 2)
                {
                    drag_cf = 2;
                }
                else
                {
                    drag_cf = value;
                }
            }
        }

        private bool slide_able = true;
        [LocalizedDescription("Slidable", typeof(WidgetManual))]
        public bool Slidable { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items1 { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items2 { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items3 { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items4 { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items5 { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items6 { get; set; }

        [LocalizedDescription("Items1to7", typeof(WidgetManual))]
        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items7 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items8 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items9 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items10 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items11 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items12 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items13 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items14 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items15 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items16 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items17 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items18 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items19 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items20 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items21 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items22 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items23 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items24 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items25 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items26 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items27 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items28 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items29 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items30 { get; set; }

        [Editor("System.Windows.Forms.Design.ListControlStringCollectionEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Content)]
        public ListBox.ObjectCollection Items31 { get; set; }

        public enum WidgetEvent
        {
            Changed = 20,
            Delay0 = 25,
            Delay1 = 26,
            Delay2 = 27,
            Delay3 = 28,
            Delay4 = 29,
            Delay5 = 30,
            Delay6 = 31,
            Delay7 = 32,
            Custom = 100
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
                        WidgetEvent e = WidgetEvent.Changed;
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

        [LocalizedDescription("Touchable", typeof(WidgetManual))]
        public bool Touchable { get; set; }

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
            ITUStepWheel stepwheel = new ITUStepWheel();

            stepwheel.type = ITUWidgetType.ITU_STEPWHEEL;
            stepwheel.name = this.Name;
            stepwheel.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;
            stepwheel.flags |= this.Touchable ? ITU.ITU_TOUCHABLE : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            stepwheel.visible = (bool)properties["Visible"].GetValue(this);

            stepwheel.active = false;
            stepwheel.dirty = false;
            stepwheel.alpha = 255;
            stepwheel.tabIndex = this.TabIndex;
            stepwheel.rect.x = this.Location.X;
            stepwheel.rect.y = this.Location.Y;
            stepwheel.rect.width = this.Size.Width;
            stepwheel.rect.height = this.Size.Height;
            stepwheel.color.alpha = this.BackAlpha;
            stepwheel.color.red = this.BackColor.R;
            stepwheel.color.green = this.BackColor.G;
            stepwheel.color.blue = this.BackColor.B;
            stepwheel.bound.x = 0;
            stepwheel.bound.y = 0;
            stepwheel.bound.width = 0;
            stepwheel.bound.height = 0;

            int index = 0;
            foreach (object item in this.Items)
            {
                ITUText text = new ITUText();

                text.flags = ITU.ITU_CLIP_DISABLED;
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

                string[] texts = new string[ITUStringSet.stringSetSize];

                texts[0] = item.ToString();
                
                if (this.Items1.Count > index)
                    texts[1] = this.Items1[index].ToString();

                if (this.Items2.Count > index)
                    texts[2] = this.Items2[index].ToString();

                if (this.Items3.Count > index)
                    texts[3] = this.Items3[index].ToString();

                if (this.Items4.Count > index)
                    texts[4] = this.Items4[index].ToString();

                if (this.Items5.Count > index)
                    texts[5] = this.Items5[index].ToString();

                if (this.Items6.Count > index)
                    texts[6] = this.Items6[index].ToString();

                if (this.Items7.Count > index)
                    texts[7] = this.Items7[index].ToString();

                if (this.Items8.Count > index)
                    texts[8] = this.Items8[index].ToString();

                if (this.Items9.Count > index)
                    texts[9] = this.Items9[index].ToString();

                if (this.Items10.Count > index)
                    texts[10] = this.Items10[index].ToString();

                if (this.Items11.Count > index)
                    texts[11] = this.Items11[index].ToString();

                if (this.Items12.Count > index)
                    texts[12] = this.Items12[index].ToString();

                if (this.Items13.Count > index)
                    texts[13] = this.Items13[index].ToString();

                if (this.Items14.Count > index)
                    texts[14] = this.Items14[index].ToString();

                if (this.Items15.Count > index)
                    texts[15] = this.Items15[index].ToString();

                if (this.Items16.Count > index)
                    texts[16] = this.Items16[index].ToString();

                if (this.Items17.Count > index)
                    texts[17] = this.Items17[index].ToString();

                if (this.Items18.Count > index)
                    texts[18] = this.Items18[index].ToString();

                if (this.Items19.Count > index)
                    texts[19] = this.Items19[index].ToString();

                if (this.Items20.Count > index)
                    texts[20] = this.Items20[index].ToString();

                if (this.Items21.Count > index)
                    texts[21] = this.Items21[index].ToString();

                if (this.Items22.Count > index)
                    texts[22] = this.Items22[index].ToString();

                if (this.Items23.Count > index)
                    texts[23] = this.Items23[index].ToString();

                if (this.Items24.Count > index)
                    texts[24] = this.Items24[index].ToString();

                if (this.Items25.Count > index)
                    texts[25] = this.Items25[index].ToString();

                if (this.Items26.Count > index)
                    texts[26] = this.Items26[index].ToString();

                if (this.Items27.Count > index)
                    texts[27] = this.Items27[index].ToString();

                if (this.Items28.Count > index)
                    texts[28] = this.Items28[index].ToString();

                if (this.Items29.Count > index)
                    texts[29] = this.Items29[index].ToString();

                if (this.Items30.Count > index)
                    texts[30] = this.Items30[index].ToString();

                if (this.Items31.Count > index)
                    texts[31] = this.Items31[index].ToString();

                text.stringSet = ITU.CreateStringSetNode(texts);

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
                WidgetNode node = new WidgetNode();
                node.widget = text;
                stepwheel.items.Add(node);
                index++;
            }

            stepwheel.focusColor.alpha = this.FocusColor.A;
            stepwheel.focusColor.red = this.FocusColor.R;
            stepwheel.focusColor.green = this.FocusColor.G;
            stepwheel.focusColor.blue = this.FocusColor.B;
            stepwheel.normalColor.alpha = this.ForeColor.A;
            stepwheel.normalColor.red = this.ForeColor.R;
            stepwheel.normalColor.green = this.ForeColor.G;
            stepwheel.normalColor.blue = this.ForeColor.B;

            stepwheel.tempy = 0;
            stepwheel.shift_one = 0;
            stepwheel.sliding = 0;
            stepwheel.scal = 0;
            stepwheel.moving_step = 0;
            stepwheel.inside = 0;
            stepwheel.slide_step = 2;  //this.Speed;
            stepwheel.slide_itemcount = 0; // this.SlideCount;
            stepwheel.idle = 0;
            stepwheel.focusIndex = this.FocusIndex;
            stepwheel.itemCount = 7; // this.ItemCount;
            stepwheel.totalframe = this.TotalFrame;



            if (this.Draggable)
                stepwheel.flags |= ITU.ITU_DRAGGABLE;

            stepwheel.fontHeight = (int)this.Font.SizeInPoints;
            stepwheel.focusFontHeight = this.FocusFontHeight;
            stepwheel.stepFontHeight1 = this.StepFontHeight1;
            stepwheel.stepFontHeight2 = this.StepFontHeight2;
            stepwheel.stepFontHeight3 = 0;

            if (this.Cycle)
                stepwheel.cycle_tor = 1; 
            else
                stepwheel.cycle_tor = 0;

            stepwheel.cycle_arr_count = 0;
            stepwheel.maxci = 0;
            stepwheel.minci = 0;
            stepwheel.layout_ci = 0;
            stepwheel.fix_count = 0;
            stepwheel.focus_c = 0;
            stepwheel.focus_dev = 0;

            if (this.FontSquare)
                stepwheel.fontsquare = 1;
            else
                stepwheel.fontsquare = 0;

            stepwheel.mouseup_change_factor = this.MouseUpChangeFactor;
            stepwheel.drag_change_factor = this.DragChangeFactor;
            stepwheel.drag_last_pos = 0;

            if (this.Slidable)
                stepwheel.slide = 1;
            else
                stepwheel.slide = 0;

            for (int i = 0; i < ITU.ITU_WHEEL_CYCLE_ARR_LIMIT; i++)
            {
                stepwheel.cycle_arr[i] = 0;
            }

            stepwheel.actions[0].action = (ITUActionType)this.Action01.Action;
            stepwheel.actions[0].ev = (ITUEvent)this.Action01.Event;
            stepwheel.actions[0].target = this.Action01.Target;
            stepwheel.actions[0].param = this.Action01.Parameter;
            stepwheel.actions[1].action = (ITUActionType)this.Action02.Action;
            stepwheel.actions[1].ev = (ITUEvent)this.Action02.Event;
            stepwheel.actions[1].target = this.Action02.Target;
            stepwheel.actions[1].param = this.Action02.Parameter;
            stepwheel.actions[2].action = (ITUActionType)this.Action03.Action;
            stepwheel.actions[2].ev = (ITUEvent)this.Action03.Event;
            stepwheel.actions[2].target = this.Action03.Target;
            stepwheel.actions[2].param = this.Action03.Parameter;
            stepwheel.actions[3].action = (ITUActionType)this.Action04.Action;
            stepwheel.actions[3].ev = (ITUEvent)this.Action04.Event;
            stepwheel.actions[3].target = this.Action04.Target;
            stepwheel.actions[3].param = this.Action04.Parameter;
            stepwheel.actions[4].action = (ITUActionType)this.Action05.Action;
            stepwheel.actions[4].ev = (ITUEvent)this.Action05.Event;
            stepwheel.actions[4].target = this.Action05.Target;
            stepwheel.actions[4].param = this.Action05.Parameter;
            stepwheel.actions[5].action = (ITUActionType)this.Action06.Action;
            stepwheel.actions[5].ev = (ITUEvent)this.Action06.Event;
            stepwheel.actions[5].target = this.Action06.Target;
            stepwheel.actions[5].param = this.Action06.Parameter;
            stepwheel.actions[6].action = (ITUActionType)this.Action07.Action;
            stepwheel.actions[6].ev = (ITUEvent)this.Action07.Event;
            stepwheel.actions[6].target = this.Action07.Target;
            stepwheel.actions[6].param = this.Action07.Parameter;
            stepwheel.actions[7].action = (ITUActionType)this.Action08.Action;
            stepwheel.actions[7].ev = (ITUEvent)this.Action08.Event;
            stepwheel.actions[7].target = this.Action08.Target;
            stepwheel.actions[7].param = this.Action08.Parameter;
            stepwheel.actions[8].action = (ITUActionType)this.Action09.Action;
            stepwheel.actions[8].ev = (ITUEvent)this.Action09.Event;
            stepwheel.actions[8].target = this.Action09.Target;
            stepwheel.actions[8].param = this.Action09.Parameter;
            stepwheel.actions[9].action = (ITUActionType)this.Action10.Action;
            stepwheel.actions[9].ev = (ITUEvent)this.Action10.Event;
            stepwheel.actions[9].target = this.Action10.Target;
            stepwheel.actions[9].param = this.Action10.Parameter;
            stepwheel.actions[10].action = (ITUActionType)this.Action11.Action;
            stepwheel.actions[10].ev = (ITUEvent)this.Action11.Event;
            stepwheel.actions[10].target = this.Action11.Target;
            stepwheel.actions[10].param = this.Action11.Parameter;
            stepwheel.actions[11].action = (ITUActionType)this.Action12.Action;
            stepwheel.actions[11].ev = (ITUEvent)this.Action12.Event;
            stepwheel.actions[11].target = this.Action12.Target;
            stepwheel.actions[11].param = this.Action12.Parameter;
            stepwheel.actions[12].action = (ITUActionType)this.Action13.Action;
            stepwheel.actions[12].ev = (ITUEvent)this.Action13.Event;
            stepwheel.actions[12].target = this.Action13.Target;
            stepwheel.actions[12].param = this.Action13.Parameter;
            stepwheel.actions[13].action = (ITUActionType)this.Action14.Action;
            stepwheel.actions[13].ev = (ITUEvent)this.Action14.Event;
            stepwheel.actions[13].target = this.Action14.Target;
            stepwheel.actions[13].param = this.Action14.Parameter;
            stepwheel.actions[14].action = (ITUActionType)this.Action15.Action;
            stepwheel.actions[14].ev = (ITUEvent)this.Action15.Event;
            stepwheel.actions[14].target = this.Action15.Target;
            stepwheel.actions[14].param = this.Action15.Parameter;

            return stepwheel;
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
