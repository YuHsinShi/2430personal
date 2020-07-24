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
    class TextBoxWidget : TextBox, IWidget
    {
        public TextBoxWidget()
        {
            base.BorderStyle = BorderStyle.None;
            base.Margin = new Padding(0, 0, 0, 0);
            base.MaxLength = 32;
            base.ForeColor = SystemColors.GrayText;
            this.TextColor = SystemColors.WindowText;
            this.FocusColor = Color.Transparent;
            this.BackAlpha = 255;
            this.LineHeight = (int)this.Font.SizeInPoints;
            this.BoldSize = 1;
            this.Font = new Font(this.Font.Name, this.Font.Size, GraphicsUnit.Pixel);
            this.Action01 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action02 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action03 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action04 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action05 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action06 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action07 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action08 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action09 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action10 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action11 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action12 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action13 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action14 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
            this.Action15 = new WidgetAction(ITU.WidgetActionType.None, WidgetEvent.MouseDown, "", "");
        }
        
        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual BorderStyle BorderStyle { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual Cursor Cursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual RightToLeft RightToLeft { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ScrollBars ScrollBars { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool UseWaitCursor { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AcceptsReturn { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool AcceptsTab { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool AllowDrop { get; set; }

        new public CharacterCasing CharacterCasing { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual ContextMenuStrip ContextMenuStrip { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool Enabled { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool HideSelection { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public ImeMode ImeMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public char PasswordChar { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public bool ReadOnly { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public virtual bool ShortcutsEnabled { get; set; }

        new public bool UseSystemPasswordChar { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public AutoCompleteStringCollection AutoCompleteCustomSource { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public AutoCompleteMode AutoCompleteMode { get; set; }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public AutoCompleteSource AutoCompleteSource { get; set; }

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
        new public ControlBindingsCollection DataBindings { get; set;  }

        [Browsable(false)]
        [DesignerSerializationVisibility(DesignerSerializationVisibility.Hidden)]
        new public object Tag { get; set; }

        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text1 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text2 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text3 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text4 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text5 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text6 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        [LocalizedDescription("Text1to7", typeof(WidgetManual))]
        public string Text7 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text8 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text9 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text10 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text11 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text12 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text13 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text14 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text15 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text16 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text17 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text18 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text19 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text20 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text21 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text22 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text23 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text24 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text25 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text26 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text27 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text28 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text29 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text30 { get; set; }
        [Editor("System.ComponentModel.Design.MultilineStringEditor, System.Design, Version=4.0.0.0, Culture=neutral, PublicKeyToken=b03f5f7f11d50a3a", typeof(UITypeEditor))]
        [Localizable(true)]
        public string Text31 { get; set; }
        [LocalizedDescription("FontIndex", typeof(WidgetManual))]
        public int FontIndex { get; set; }
        [LocalizedDescription("BoldSize", typeof(WidgetManual))]
        public int BoldSize { get; set; }
        [LocalizedDescription("ShowCursor", typeof(WidgetManual))]
        public bool ShowCursor { get; set; }
        [LocalizedDescription("TextColor", typeof(WidgetManual))]
        public Color TextColor { get; set; }
        [LocalizedDescription("BackAlpha", typeof(WidgetManual))]
        public byte BackAlpha { get; set; }
        [LocalizedDescription("LineHeight", typeof(WidgetManual))]
        public int LineHeight { get; set; }
        [LocalizedDescription("FocusColor", typeof(WidgetManual))]
        public Color FocusColor { get; set; }
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

        public enum WidgetEvent
        {
            Timer = 0,
            Press = 1,
            KeyDown = 4,
            MouseDown = 6,
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
                        WidgetEvent e = WidgetEvent.MouseDown;
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
            ITUTextBox widget = new ITUTextBox();

            widget.name = this.Name;
            widget.flags |= this.TabStop ? ITU.ITU_TAPSTOP : 0;

            PropertyDescriptorCollection properties = TypeDescriptor.GetProperties(this);
            widget.visible = (bool)properties["Visible"].GetValue(this);

            widget.active = false;
            widget.dirty = false;
            widget.alpha = 255;
            widget.tabIndex = this.TabIndex;
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
            widget.bgColor.alpha = this.BackAlpha;
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

            switch (this.TextAlign)
            {
                case HorizontalAlignment.Center:
                    widget.layout = ITULayout.ITU_LAYOUT_MIDDLE_CENTER;
                    break;

                case HorizontalAlignment.Right:
                    widget.layout = ITULayout.ITU_LAYOUT_MIDDLE_RIGHT;
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

            widget.maxLen = this.MaxLength;
            widget.textboxFlags = 0;
            
            if (this.UseSystemPasswordChar)
                widget.textboxFlags |= ITUTextBox.ITU_TEXTBOX_PASSWORD;

            if (this.ShowCursor)
                widget.textboxFlags |= ITUTextBox.ITU_TEXTBOX_CURSOR;

            if (this.CharacterCasing == CharacterCasing.Upper)
                widget.textboxFlags |= ITUTextBox.ITU_TEXTBOX_UPPER;

            if (this.CharacterCasing == CharacterCasing.Lower)
                widget.textboxFlags |= ITUTextBox.ITU_TEXTBOX_LOWER;

            if (this.Multiline)
                widget.textboxFlags |= ITUTextBox.ITU_TEXTBOX_MULTILINE;

            if (this.WordWrap)
                widget.textboxFlags |= ITUTextBox.ITU_TEXTBOX_WORDWRAP;

            widget.fgColor.alpha = this.TextColor.A;
            widget.fgColor.red = this.TextColor.R;
            widget.fgColor.green = this.TextColor.G;
            widget.fgColor.blue = this.TextColor.B;
            widget.defColor.alpha = this.ForeColor.A;
            widget.defColor.red = this.ForeColor.R;
            widget.defColor.green = this.ForeColor.G;
            widget.defColor.blue = this.ForeColor.B;
            widget.focusColor.alpha = this.FocusColor.A;
            widget.focusColor.red = this.FocusColor.R;
            widget.focusColor.green = this.FocusColor.G;
            widget.focusColor.blue = this.FocusColor.B;

            widget.lineHeight = this.LineHeight;
            widget.letterSpacing = this.LetterSpacing;

            widget.actions[0].action = (ITUActionType)this.Action01.Action;
            widget.actions[0].ev = (ITUEvent)this.Action01.Event;
            widget.actions[0].target = this.Action01.Target;
            widget.actions[0].param = this.Action01.Parameter;
            widget.actions[1].action = (ITUActionType)this.Action02.Action;
            widget.actions[1].ev = (ITUEvent)this.Action02.Event;
            widget.actions[1].target = this.Action02.Target;
            widget.actions[1].param = this.Action02.Parameter;
            widget.actions[2].action = (ITUActionType)this.Action03.Action;
            widget.actions[2].ev = (ITUEvent)this.Action03.Event;
            widget.actions[2].target = this.Action03.Target;
            widget.actions[2].param = this.Action03.Parameter;
            widget.actions[3].action = (ITUActionType)this.Action04.Action;
            widget.actions[3].ev = (ITUEvent)this.Action04.Event;
            widget.actions[3].target = this.Action04.Target;
            widget.actions[3].param = this.Action04.Parameter;
            widget.actions[4].action = (ITUActionType)this.Action05.Action;
            widget.actions[4].ev = (ITUEvent)this.Action05.Event;
            widget.actions[4].target = this.Action05.Target;
            widget.actions[4].param = this.Action05.Parameter;
            widget.actions[5].action = (ITUActionType)this.Action06.Action;
            widget.actions[5].ev = (ITUEvent)this.Action06.Event;
            widget.actions[5].target = this.Action06.Target;
            widget.actions[5].param = this.Action06.Parameter;
            widget.actions[6].action = (ITUActionType)this.Action07.Action;
            widget.actions[6].ev = (ITUEvent)this.Action07.Event;
            widget.actions[6].target = this.Action07.Target;
            widget.actions[6].param = this.Action07.Parameter;
            widget.actions[7].action = (ITUActionType)this.Action08.Action;
            widget.actions[7].ev = (ITUEvent)this.Action08.Event;
            widget.actions[7].target = this.Action08.Target;
            widget.actions[7].param = this.Action08.Parameter;
            widget.actions[8].action = (ITUActionType)this.Action09.Action;
            widget.actions[8].ev = (ITUEvent)this.Action09.Event;
            widget.actions[8].target = this.Action09.Target;
            widget.actions[8].param = this.Action09.Parameter;
            widget.actions[9].action = (ITUActionType)this.Action10.Action;
            widget.actions[9].ev = (ITUEvent)this.Action10.Event;
            widget.actions[9].target = this.Action10.Target;
            widget.actions[9].param = this.Action10.Parameter;
            widget.actions[10].action = (ITUActionType)this.Action11.Action;
            widget.actions[10].ev = (ITUEvent)this.Action11.Event;
            widget.actions[10].target = this.Action11.Target;
            widget.actions[10].param = this.Action11.Parameter;
            widget.actions[11].action = (ITUActionType)this.Action12.Action;
            widget.actions[11].ev = (ITUEvent)this.Action12.Event;
            widget.actions[11].target = this.Action12.Target;
            widget.actions[11].param = this.Action12.Parameter;
            widget.actions[12].action = (ITUActionType)this.Action13.Action;
            widget.actions[12].ev = (ITUEvent)this.Action13.Event;
            widget.actions[12].target = this.Action13.Target;
            widget.actions[12].param = this.Action13.Parameter;
            widget.actions[13].action = (ITUActionType)this.Action14.Action;
            widget.actions[13].ev = (ITUEvent)this.Action14.Event;
            widget.actions[13].target = this.Action14.Target;
            widget.actions[13].param = this.Action14.Parameter;
            widget.actions[14].action = (ITUActionType)this.Action15.Action;
            widget.actions[14].ev = (ITUEvent)this.Action15.Event;
            widget.actions[14].target = this.Action15.Target;
            widget.actions[14].param = this.Action15.Parameter;

            return widget;
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
