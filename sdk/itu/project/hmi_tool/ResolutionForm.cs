using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Windows.Forms;

namespace GUIDesigner
{
    public partial class ResolutionForm : Form
    {
        public ResolutionForm()
        {
            InitializeComponent();
            this.widthNumericUpDown.Value = ITU.screenWidth;
            this.heightNumericUpDown.Value = ITU.screenHeight;
        }

        private void okButton_Click(object sender, EventArgs e)
        {
            MainForm.mainForm.ChangeResolution((int)this.widthNumericUpDown.Value, (int)this.heightNumericUpDown.Value);
            this.Close();
        }
    }
}
