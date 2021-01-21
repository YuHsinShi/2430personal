using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.ComponentModel.Design;
using System.ComponentModel.Design.Serialization;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Drawing.Design;
using System.Runtime.InteropServices;
using System.IO;
using System.Linq;
using System.Text;
using System.Windows.Forms;
using System.Windows.Forms.Design;
using System.Xml;
using System.Drawing.Imaging;
using System.Management;
using Microsoft.Win32;
using SharpFont;

namespace GUIDesigner
{
    public partial class MainForm : Form
    {
        static public MainForm mainForm;

        private String xmlPath;
        private String ituPath;
        private String fontPath;
        private String imagePath;
        private String buildPath;
        private const String emulatorPath = "itu_emulator.exe";
        private const String defaultFontName = "WenQuanYiMicroHeiMono.ttf";
        private bool refreshingTreeView = false;
        private Timer refreshTreeViewTimer = new Timer();
        private string focusedTreeNodeName = null;
        private HostSurface currHostSurface = null;
        bool treeViewSelecting = false;

        //Bless added for auto regen
        public int MAX_ITU_AUTOGEN_JOB_PERTIME = 1;
        public const string xml_father_path = "D:\\ITE_Castor3\\";
        public static System.Threading.Mutex mpg;

        //Bless added for SPECIAL USAGE
        public const int itu_mode = 0; //0 for us, 1 for HMI
        public String special_working_path = "";

        //Bless added for Action copy
        private string object_to_copy_action = "";
        private object[] action_copy_str = new object[15];
        private struct copy_WidgetAction
        {
            public ITU.WidgetActionType Action { get; set; }
            public int Event { get; set; }
            public String Target { get; set; }
            public String Parameter { get; set; }
        }

        //Bless added for autosearch xml
        public static int autosearchXML(string path, string[] todoStr)
        {
            int i = 0;
            // Specify the directory you want to manipulate.
            //string path = "a:\\";
            string searchPattern = "*.xml";

            DirectoryInfo di = new DirectoryInfo(path);

            FileInfo[] files =
                di.GetFiles(searchPattern, SearchOption.AllDirectories);

            foreach (FileInfo file in files)
            {
                int length = file.FullName.Length;
                string existituname = file.FullName.Substring(0, length - 3) + "itu";

                if (File.Exists(existituname))
                {
                    //Console.WriteLine("{0,-25} {1,25}", file.FullName, existituname);
                    todoStr[i] = file.FullName;
                    i++;
                }
            }

            return i;
        }

        public MainForm(string value)
        {
            //Bless added for auto regen
            int allfilecount = 0;
            string[] allfilestr = new string[500];
            bool useFullAuto = false;
            string searchrootfile = System.Windows.Forms.Application.StartupPath + "\\bless_auto.txt";
            string searchrootdir;

            bool checkSingle;
            this.max_regen_item = 500;
            this.itu_regen_ini = System.Windows.Forms.Application.StartupPath + "\\itu_regen.ini";
            System.Threading.Thread thd = new System.Threading.Thread(_Thread_AutoGenITU);

            ITU.fontLib = new Library();

            WidgetLoader.layerWidgetType = typeof(LayerWidget);
            MainForm.mainForm = this;

            InitializeComponent();
            InitializeToolbox();
            InitializeDesignerSurface(null, null);
            ITU.layerCount = this.layerTabControl.TabCount;
            Application.AddMessageFilter(new KeystrokMessageFilter());
            Application.AddMessageFilter(new MouseMessageFilter(this.statusLabel));
            //DraggableTabControl.SetHostSurfaces(this.hostSurfaces);

            widgetListBox.Items.Clear();
            AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

            widgetTreeView.Nodes.Clear();
            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);

            FileInfo fInfo = new FileInfo(defaultFontName);
            if (fInfo.Exists)
            {
                fontPath = fInfo.FullName;
                ITU.fontFace = new Face(ITU.fontLib, fontPath);
            }
            else
            {
                fInfo = new FileInfo("../../../../data/font/" + defaultFontName);
                if (fInfo.Exists)
                {
                    fontPath = fInfo.FullName;
                    ITU.fontFace = new Face(ITU.fontLib, fontPath);
                }
            }
            ITU.layerWidget = GetLayerWidget(this.layerTabControl.SelectedTab);

            refreshTreeViewTimer.Interval = 100;
            refreshTreeViewTimer.Tick += new EventHandler(refreshTreeViewTimer_Tick);

            widgetTreeContextMenuStrip.Opening += new CancelEventHandler(widgetTreeContextMenuStrip_Opening);
            widgetTreeContextMenuStrip.Opened += new EventHandler(widgetTreeContextMenuStrip_Opened);

            mpg = new System.Threading.Mutex(false, "GuidesignerIsUnique", out checkSingle);
            //mpg.WaitOne();

            //modify the description panel of PropertyGrid
            foreach (Control control in this.designerPropertyGrid.Controls)
            {
                if (control.GetType().Name == "DocComment")
                {
                    System.Reflection.FieldInfo fieldInfo = control.GetType().BaseType.GetField("userSized",
                      System.Reflection.BindingFlags.Instance |
                      System.Reflection.BindingFlags.NonPublic);
                    fieldInfo.SetValue(control, true);
                    control.Height = control.Height + 80;
                }
            }

            if (itu_mode == 1)
            {
                this.xmlPath = "";
                //this.xmlPath = "d:\\best01.xml";
                //value = this.xmlPath;
                if (!value.Equals("0"))
                {
                    this.xmlPath = value;
                    thd.Priority = System.Threading.ThreadPriority.Highest;
                    thd.Start();
                }
                else
                {
                    //MessageBox.Show("usage: xml_to_itu.exe <full xml filename ex: d:\\123.xml>", "error usage");
                    //Console.WriteLine("usage: xml_to_itu.exe <full xml filename ex: d:\\123.xml>");
                    mpg.Dispose();
                    Environment.Exit(Environment.ExitCode);
                }

                /*
                if (!value.Equals("0"))
                {
                    string section;
                    XmlDocument doc = new XmlDocument();
                    doc.AppendChild(doc.CreateXmlDeclaration("1.0", "UTF-8", "yes"));
                    XmlElement rootElement = doc.CreateElement("ITURegenWork");
                    doc.AppendChild(rootElement);
                    this.max_regen_item = 1;

                    for (int i = 0; i < this.max_regen_item; i++)
                    {
                        string filepath = "";
                        section = "JOB" + i.ToString();
                        XmlElement Child = doc.CreateElement(section);

                        switch (i)
                        {
                            case 0:
                                filepath = value;
                                break;
                            default:
                                filepath = "C:\\NULL.xml";
                                break;
                        }

                        XmlAttribute attChild1 = doc.CreateAttribute("XMLFILE");
                        XmlAttribute attChild2 = doc.CreateAttribute("WORK");
                        XmlAttribute attChild3 = doc.CreateAttribute("JPEGQT");

                        attChild1.Value = filepath;
                        attChild2.Value = "1";
                        attChild3.Value = "75";

                        Child.Attributes.Append(attChild1);
                        Child.Attributes.Append(attChild2);
                        Child.Attributes.Append(attChild3);

                        rootElement.AppendChild(Child);
                    }

                    doc.Save(itu_regen_ini);
                }
                */
            }
            else
            {
                if (!value.Equals("0"))
                {
                    this.xmlPath = value;
                    //MessageBox.Show(value);
                }
            }

            //Bless added for file fully auto regen
            if ((!useFullAuto) && File.Exists(searchrootfile))
            {
                System.IO.StreamReader file = new System.IO.StreamReader(searchrootfile);
                searchrootdir = file.ReadLine();

                if (searchrootdir.Substring(searchrootdir.Length) != "\\")
                    searchrootdir += "\\";

                if (searchrootdir != null)
                {
                    useFullAuto = true;

                    allfilecount = autosearchXML(searchrootdir, allfilestr);

                    file.Close();
                    File.Delete(searchrootfile);
                }
            }

            if (useFullAuto)
            {
                string section;
                XmlDocument doc = new XmlDocument();
                doc.AppendChild(doc.CreateXmlDeclaration("1.0", "UTF-8", "yes"));
                XmlElement rootElement = doc.CreateElement("ITURegenWork");
                doc.AppendChild(rootElement);

                if (File.Exists(itu_regen_ini))
                    File.Delete(itu_regen_ini);

                for (int i = 0; i < this.max_regen_item; i++)
                {
                    string filepath = "";
                    section = "JOB" + i.ToString();
                    XmlElement Child = doc.CreateElement(section);

                    if (i >= allfilecount)
                        break;

                    filepath = allfilestr[i];

                    XmlAttribute attChild1 = doc.CreateAttribute("XMLFILE");
                    XmlAttribute attChild2 = doc.CreateAttribute("WORK");
                    XmlAttribute attChild3 = doc.CreateAttribute("JPEGQT");

                    attChild1.Value = filepath;
                    attChild2.Value = "1";
                    attChild3.Value = "75";

                    Child.Attributes.Append(attChild1);
                    Child.Attributes.Append(attChild2);
                    Child.Attributes.Append(attChild3);

                    rootElement.AppendChild(Child);
                }
                doc.Save(itu_regen_ini);
            }

            if (File.Exists(itu_regen_ini) && (itu_mode == 0))
            {
                int workquene = 0;
                string section = "";

                for (int i = 0; i < this.max_regen_item; i++)
                {
                    string wkstr = "";
                    section = "JOB" + i.ToString();
                    XmlDocument doc = new XmlDocument();
                    XmlNode rootnode = null;
                    doc.Load(itu_regen_ini);
                    rootnode = doc.DocumentElement;
                    foreach (XmlNode node in rootnode.ChildNodes)
                    {
                        if (node.Name == section)
                        {
                            wkstr = node.Attributes.GetNamedItem("WORK").Value;
                            if (wkstr == "1")
                                workquene++;
                        }
                    }
                }

                if (workquene > 0)
                {
                    if (!checkSingle)
                    {
                        mpg.Dispose();
                        mpg = new System.Threading.Mutex(false, "GuidesignerIsUnique", out checkSingle);
                    }

                    if (!checkSingle)
                    {
                        //MessageBox.Show("111", "111");
                        Environment.Exit(Environment.ExitCode);
                    }
                    else
                    {
                        //MessageBox.Show("222", "222");
                        thd.Priority = System.Threading.ThreadPriority.Highest;
                        thd.Start();
                    }
                }
                else if (itu_mode == 1)
                {
                    Environment.Exit(Environment.ExitCode);
                }
                else
                {
                    this.WindowState = FormWindowState.Normal;
                    this.Show();
                }

            }
            else if (itu_mode == 0)
            {
                string section;
                XmlDocument doc = new XmlDocument();
                doc.AppendChild(doc.CreateXmlDeclaration("1.0", "UTF-8", "yes"));
                XmlElement rootElement = doc.CreateElement("ITURegenWork");
                doc.AppendChild(rootElement);

                if (itu_mode == 1) //for HMI mode
                {
                    string working_path = System.Windows.Forms.Application.StartupPath + "\\";
                    this.max_regen_item = 1;

                    for (int i = 0; i < this.max_regen_item; i++)
                    {
                        string filepath = "";
                        section = "JOB" + i.ToString();
                        XmlElement Child = doc.CreateElement(section);

                        switch (i)
                        {
                            case 0:
                                filepath = working_path + "hmi.xml";
                                break;
                            default:
                                filepath = "C:\\NULL.xml";
                                break;
                        }

                        XmlAttribute attChild1 = doc.CreateAttribute("XMLFILE");
                        XmlAttribute attChild2 = doc.CreateAttribute("WORK");
                        XmlAttribute attChild3 = doc.CreateAttribute("JPEGQT");

                        attChild1.Value = filepath;
                        attChild2.Value = "0";
                        attChild3.Value = "75";

                        Child.Attributes.Append(attChild1);
                        Child.Attributes.Append(attChild2);
                        Child.Attributes.Append(attChild3);

                        rootElement.AppendChild(Child);
                    }
                }
                else
                {
                    for (int i = 0; i < this.max_regen_item; i++)
                    {
                        string filepath = "";
                        section = "JOB" + i.ToString();
                        XmlElement Child = doc.CreateElement(section);

                        switch (i)
                        {
                            case 0:
                                filepath = xml_father_path + "project\\ctrlboard\\itu\\480x272\\ctrlboard.xml";
                                break;
                            case 1:
                                filepath = xml_father_path + "project\\ctrlboard\\itu\\480x272\\upgrade.xml";
                                break;
                            case 2:
                                filepath = xml_father_path + "project\\ctrlboard\\itu\\480x272_rotate\\ctrlboard.xml";
                                break;
                            case 3:
                                filepath = xml_father_path + "project\\ctrlboard\\itu\\800x480\\ctrlboard.xml";
                                break;
                            case 4:
                                filepath = xml_father_path + "project\\ctrlboard\\itu\\800x480\\upgrade.xml";
                                break;
                            case 5:
                                filepath = xml_father_path + "project\\ctrlboard\\itu\\800x480_rotate\\ctrlboard.xml";
                                break;
                            case 6:
                                filepath = xml_father_path + "project\\doorbell_admin\\itu\\doorbell_admin.xml";
                                break;
                            case 7:
                                filepath = xml_father_path + "project\\doorbell_indoor2\\itu\\800x480\\doorbell_indoor.xml";
                                break;
                            case 8:
                                filepath = xml_father_path + "project\\doorbell_indoor2\\itu\\1024x600\\doorbell_indoor.xml";
                                break;
                            case 9:
                                filepath = xml_father_path + "project\\doorbell_indoor2\\itu\\1280x800\\doorbell_indoor.xml";
                                break;
                            default:
                                filepath = "C:\\NULL.xml";
                                break;
                        }

                        XmlAttribute attChild1 = doc.CreateAttribute("XMLFILE");
                        XmlAttribute attChild2 = doc.CreateAttribute("WORK");
                        XmlAttribute attChild3 = doc.CreateAttribute("JPEGQT");

                        attChild1.Value = filepath;
                        attChild2.Value = "0";
                        attChild3.Value = "75";

                        Child.Attributes.Append(attChild1);
                        Child.Attributes.Append(attChild2);
                        Child.Attributes.Append(attChild3);

                        rootElement.AppendChild(Child);
                    }
                }
                doc.Save(itu_regen_ini);

                if (itu_mode == 1)
                {
                    Environment.Exit(Environment.ExitCode);
                }
                else
                {
                    this.WindowState = FormWindowState.Normal;
                    this.Show();
                }
            }

            //mpg.ReleaseMutex();
        }

        public delegate void myitugendalegate();

        private void _ThreadSafe_For_AutoGenITU()
        {
            try
            {
                HostSurface.hostSurfaces.Clear();
                this.layerTabControl.TabPages.Clear();
                this.layerListBox.Items.Clear();
                NameCreationService.Reset();
                ITU.layerWidget = null;

                for (int i = 0; i < GC.MaxGeneration; i++)
                {
                    GC.Collect(i);
                    GC.WaitForPendingFinalizers();
                }

                LoadFile(this.xmlPath);

                foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
                {
                    UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                    ues.Reset();
                }

                StringBuilder sb;
                int index = this.xmlPath.LastIndexOf('.');
                if (index == -1)
                    sb = new StringBuilder(this.xmlPath);
                else
                    sb = new StringBuilder(this.xmlPath, 0, index, 512);

                sb.Append(".itu");

                this.ituPath = sb.ToString();
                ITU.modified = true;
                this.Text = "GUI Designer - " + this.xmlPath;

                widgetListBox.Items.Clear();
                AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

                widgetTreeView.Nodes.Clear();
                AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

                ITU.layerWidget = GetLayerWidget(this.layerTabControl.SelectedTab);
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
            }

            ITU.jpegCompression = jpegCompressionToolStripMenuItem.Checked;
            ITU.jpegAlphaCompression = jpegAlphaCompressionToolStripMenuItem.Checked;
            ITU.bigEndian = bigEndianToolStripMenuItem.Checked;
            ITU.speedyCompression = speedyCompressionToolStripMenuItem.Checked;
            ITU.speedySurfaceCompression = speedySurfaceCompressionToolStripMenuItem.Checked;
            ITU.SaveItu(this.ituPath, compressionToolStripMenuItem.Checked, itu_mode);

            if (itu_mode == 0)
            {
                this.itu_node.Attributes.GetNamedItem("WORK").Value = "0";
                this.itu_doc.Save(this.itu_regen_ini);
            }
            else
            {
                Environment.Exit(Environment.ExitCode);
            }
        }

        private void _Thread_AutoGenITU()
        {
            //MessageBox.Show("Found AutoGen ITU Job...Start to Work!", "Start To Autogen ITU");
            if (itu_mode == 1)
            {
                System.Threading.Thread.Sleep(1000);

                ITU.jpegCompression = false;
                ITU.jpegQuality = 75;

                if (File.Exists(this.xmlPath))
                {
                    myitugendalegate md = new myitugendalegate(_ThreadSafe_For_AutoGenITU);

                    this.Invoke(md);
                }
            }
            else if (itu_mode == 0)
            {
                FormAutoGenITU ms = new FormAutoGenITU();
                ms.label1.Text = "Start To Autogen ITU";
                ms.label2.Text = "Preparing...";
                ms.Show();
                ms.Refresh();
                System.Threading.Thread.Sleep(3000);

                int maxcount = 0;
                int maxjob = MAX_ITU_AUTOGEN_JOB_PERTIME;
                int workquene = 0;
                string section = "";

                for (int i = 0; i < this.max_regen_item; i++)
                {
                    string wkstr = "";
                    section = "JOB" + i.ToString();
                    XmlDocument doc = new XmlDocument();
                    XmlNode rootnode = null;
                    doc.Load(itu_regen_ini);
                    rootnode = doc.DocumentElement;
                    foreach (XmlNode node in rootnode.ChildNodes)
                    {
                        if (node.Name == section)
                        {
                            wkstr = node.Attributes.GetNamedItem("WORK").Value;
                            if (wkstr == "1")
                                workquene++;
                        }
                    }
                }

                for (int i = 0; i < this.max_regen_item; i++)
                {
                    string wkstr = "";
                    string xmltoregen = "";
                    string jpegqual = "";
                    section = "JOB" + i.ToString();
                    XmlDocument doc = new XmlDocument();
                    XmlNode rootnode = null;
                    XmlNode worknode = null;
                    doc.Load(itu_regen_ini);
                    rootnode = doc.DocumentElement;
                    foreach (XmlNode node in rootnode.ChildNodes)
                    {
                        if (node.Name == section)
                        {
                            worknode = node;
                            wkstr = node.Attributes.GetNamedItem("WORK").Value;
                            xmltoregen = node.Attributes.GetNamedItem("XMLFILE").Value;
                            jpegqual = node.Attributes.GetNamedItem("JPEGQT").Value;

                            this.itu_node = worknode;
                            this.itu_doc = doc;
                            break;
                        }
                    }

                    if (wkstr == "1")
                    {
                        string xmltowork = xmltoregen;
                        int jpegqualvalue = 0;
                        bool canConvert = int.TryParse(jpegqual, out jpegqualvalue);

                        if (canConvert)
                        {
                            if ((jpegqualvalue > 0) && (jpegqualvalue < 100))
                            {
                                ITU.jpegCompression = true;
                                ITU.jpegQuality = jpegqualvalue;
                            }
                            else
                            {
                                ITU.jpegCompression = false;
                                ITU.jpegQuality = 75;
                            }
                        }

                        //int jpegqualvalue = jpegqual.IsNormalized

                        if (File.Exists(xmltowork))
                        {
                            FileInfo FI = new FileInfo(xmltowork);
                            this.xmlPath = xmltowork;
                            int curritem = maxcount + 1;
                            long xmlsize = FI.Length;

                            ms.label1.Text = "Start To Autogen ITU (" + curritem.ToString() + "/" + workquene.ToString() + ")";
                            ms.label2.Text = xmltowork + "  (" + xmlsize.ToString() + ")";
                            ms.Refresh();
                            System.Threading.Thread.Sleep(1000);

                            myitugendalegate md = new myitugendalegate(_ThreadSafe_For_AutoGenITU);

                            this.Invoke(md);

                            maxcount++;
                        }//if (File.Exists(xmltowork))
                    }

                    if ((maxcount >= maxjob) && (workquene > maxjob))
                    {
                        System.Diagnostics.ProcessStartInfo startInfo = new System.Diagnostics.ProcessStartInfo();
                        startInfo.UseShellExecute = true;
                        startInfo.WorkingDirectory = Environment.CurrentDirectory;
                        startInfo.FileName = Application.ExecutablePath;
                        startInfo.Verb = "runas";

                        ms.label1.Text = "Max " + maxjob.ToString() + " jobs perform one time....Auto restart to continue now!";
                        ms.label2.Text = "";
                        ms.Refresh();
                        System.Threading.Thread.Sleep(1000);
                        //System.Diagnostics.Process.Start(Application.ExecutablePath);
                        System.Diagnostics.Process.Start(startInfo);
                        Environment.Exit(Environment.ExitCode);
                    }
                }

                if ((maxcount == workquene) && (workquene > 0))
                {
                    //MessageBox.Show("All auto regen itu job is done!", "Job Finished!");
                    ms.label1.Text = "All auto regen itu job is done!";
                    ms.label2.Text = "";
                    ms.Refresh();
                    System.Threading.Thread.Sleep(2000);

                    Environment.Exit(Environment.ExitCode);
                }
            }
        }

        void widgetTreeContextMenuStrip_Opening(object sender, CancelEventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is LayerWidget)
                e.Cancel = true;
        }

        void widgetTreeContextMenuStrip_Opened(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is IWidget)
            {
                IWidget widget = treeNode.Tag as IWidget;

                ToolStripMenuItem item = widgetTreeContextMenuStrip.Items["showToolStripMenuItem"] as ToolStripMenuItem;
                item.Checked = !widget.Hided;

                item = widgetTreeContextMenuStrip.Items["hideToolStripMenuItem"] as ToolStripMenuItem;
                item.Checked = widget.Hided;
            }
        }

        void refreshTreeViewTimer_Tick(object sender, EventArgs e)
        {
            widgetTreeView.Nodes.Clear();
            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

            if (focusedTreeNodeName != null)
            {
                TreeNode[] nodes = this.widgetTreeView.Nodes.Find(focusedTreeNodeName, true);
                if (nodes.Length > 0)
                {
                    widgetTreeView.SelectedNode = nodes[0];
                    nodes[0].EnsureVisible();
                    widgetTreeView.Focus();
                }
                focusedTreeNodeName = null;
            }
            refreshingTreeView = false;
            refreshTreeViewTimer.Enabled = false;
        }

        private void InitializeDesignerSurface(XmlDocument doc, XmlElement element)
        {
            HostSurface hostSurface = new HostSurface();

            IServiceContainer serviceProvider = hostSurface.GetService(typeof(IServiceContainer)) as IServiceContainer;
            DesignerOptionService opsService = serviceProvider.GetService(typeof(DesignerOptionService)) as DesignerOptionService;
            if (null != opsService)
            {
                serviceProvider.RemoveService(typeof(DesignerOptionService));
            }
            DesignerOptionService opsService2 = new DesignerOptionServiceImpl();
            serviceProvider.AddService(typeof(DesignerOptionService), opsService2);

            hostSurface.Load(doc, element);
            Control view = hostSurface.View as Control;
            LayerWidget rootWidget = hostSurface.formDesignerHost.RootComponent as LayerWidget;
            TabPage tp = new TabPage(rootWidget.Name);

            int index = -1;
            for (int i = 0; i < this.layerTabControl.TabPages.Count; i++)
            {
                if (this.layerTabControl.TabPages[i].Text == rootWidget.Name)
                {
                    index = i;
                    break;
                }
            }
            if (index >= 0)
            {
                HostSurface.hostSurfaces.RemoveAt(index);
                this.layerTabControl.TabPages.RemoveAt(index);
                this.layerTabControl.TabPages.Insert(index, tp);
                HostSurface.hostSurfaces.Insert(index, hostSurface);
            }
            else
            {
                this.layerTabControl.TabPages.Add(tp);
                this.layerListBox.Items.Add(rootWidget.Name);
                HostSurface.hostSurfaces.Add(hostSurface);
            }


            view.Dock = DockStyle.Fill;
            view.Parent = tp;

            tp.Enter += new EventHandler(tp_Enter);

            ISelectionService selectionService = hostSurface.formDesignerHost.GetService(typeof(ISelectionService)) as ISelectionService;
            selectionService.SelectionChanged += delegate
            {
                ISelectionService s = (ISelectionService)hostSurface.GetService(typeof(ISelectionService));

                object[] selection;
                if (s.SelectionCount == 0)
                    this.designerPropertyGrid.SelectedObject = null;
                else
                {
                    selection = new object[s.SelectionCount];
                    s.GetSelectedComponents().CopyTo(selection, 0);
                    this.designerPropertyGrid.SelectedObjects = selection;
                    
                    string t = s.PrimarySelection.GetType().ToString();
                    t = t.Substring(12);
                    this.toolStripStatusLabel1.Text = t.Remove(t.Length - 6);

                    Control ctrl = s.PrimarySelection as Control;
                    if (ctrl != null)
                    {
                        this.designerPropertyGrid.Site = ctrl.Site;
                        if (ctrl is IWidget && !treeViewSelecting)
                        {
                            focusedTreeNodeName = ctrl.Name;
                            RefreshTreeView();
                        }
                        Point loc = ctrl.FindForm().PointToClient(ctrl.Parent.PointToScreen(ctrl.Location));
                        this.toolStripStatusLabel1.Text += " " + loc;
                    }
                }
            };

            IComponentChangeService changeService = hostSurface.formDesignerHost.GetService(typeof(IComponentChangeService)) as IComponentChangeService;
            changeService.ComponentAdded += new ComponentEventHandler(changeService_ComponentAdded);
            changeService.ComponentRemoving += new ComponentEventHandler(changeService_ComponentRemoving);
            changeService.ComponentRename += new ComponentRenameEventHandler(changeService_ComponentRename);
            changeService.ComponentChanged += new ComponentChangedEventHandler(changeService_ComponentChanged);

            view.AllowDrop = true;
            view.DragDrop += delegate(object sender, DragEventArgs e)
            {
                if (!e.Data.GetDataPresent(typeof(ToolboxItem)))
                {
                    e.Effect = DragDropEffects.None;
                    return;
                }
                ToolboxItem item = e.Data.GetData(typeof(ToolboxItem)) as ToolboxItem;
                e.Effect = DragDropEffects.Copy;
                item.CreateComponents(hostSurface.formDesignerHost);
            };

            ToolboxService toolboxService = new ToolboxService();
            toolboxService.toolboxListBox = toolboxListBox;
            hostSurface.formDesignerHost.AddService(typeof(IToolboxService), toolboxService);

            KeystrokMessageFilter.host = hostSurface.formDesignerHost;
            MouseMessageFilter.view = rootWidget;

            Control c = rootWidget as Control;
            ITU.screenWidth = c.Size.Width;
            ITU.screenHeight = c.Size.Height;
        }

        private void RefreshTreeView()
        {
            if (refreshingTreeView)
                return;

            refreshTreeViewTimer.Enabled = true;
            refreshingTreeView = true;
        }

        void changeService_ComponentAdded(object sender, ComponentEventArgs e)
        {
            if (e.Component is IWidget)
            {
                this.widgetListBox.Items.Add(e.Component.Site.Name);
                focusedTreeNodeName = e.Component.Site.Name;
                RefreshTreeView();
            }
        }

        void changeService_ComponentRemoving(object sender, ComponentEventArgs e)
        {
            if (e.Component is IWidget)
            {
                NameCreationService.names.Remove(e.Component.Site.Name);
                this.widgetListBox.Items.Remove(e.Component.Site.Name);

                TreeNode[] nodes = this.widgetTreeView.Nodes.Find(e.Component.Site.Name, true);
                if (nodes.Length > 0)
                    this.widgetTreeView.Nodes.Remove(nodes[0]);
            }
        }

        void changeService_ComponentRename(object sender, ComponentRenameEventArgs e)
        {
            if (e.Component is LayerWidget)
            {
                this.layerTabControl.TabPages[this.layerTabControl.SelectedIndex].Text = e.NewName;

                this.layerListBox.Items.Remove(e.OldName);
                this.layerListBox.Items.Add(e.NewName);
            }

            if (e.Component is IWidget)
            {
                NameCreationService.names.Remove(e.OldName);
                NameCreationService.names.Add(e.NewName);

                this.widgetListBox.Items.Remove(e.OldName);
                this.widgetListBox.Items.Add(e.NewName);

                TreeNode[] nodes = this.widgetTreeView.Nodes.Find(e.OldName, true);
                if (nodes.Length > 0)
                {
                    nodes[0].Name = e.NewName;
                    nodes[0].Text = e.NewName;
                }
            }
        }

        private TreeNode FindWidgetTreeNode(Control c, TreeNodeCollection nodes)
        {
            TreeNode result = null;

            foreach (TreeNode node in nodes)
            {
                if (node.Tag == c)
                {
                    result = node;
                    break;
                }
                else
                {
                    result = FindWidgetTreeNode(c, node.Nodes);
                    if (result != null)
                        break;
                }
            }
            return result;
        }

        void changeService_ComponentChanged(object sender, ComponentChangedEventArgs e)
        {
            if (e.Component is IWidget && e.Component is Control)
            {
                Control c = e.Component as Control;
                if (c.Parent != null && e.Member != null)
                {
                    if (e.Member.Name.Equals("Location"))
                    {
                        TreeNode parent = FindWidgetTreeNode(c.Parent, widgetTreeView.Nodes);
                        if (parent != null)
                        {
                            TreeNode child = FindWidgetTreeNode(c, widgetTreeView.Nodes);

                            if (parent.Tag != c.Parent || child == null || child.Parent != parent)
                            {
                                //Force the StopAnywhere only one child (Bless)
                                Type type = c.Parent.GetType();

                                if (type == typeof(StopAnywhereWidget))
                                {
                                    int count = parent.GetNodeCount(false);

                                    if (count > 0)
                                        return;
                                }

                                if (child != null)
                                {
                                    child.Remove();
                                    parent.Nodes.Add(child);
                                }
                                else
                                {
                                    child = parent.Nodes.Add(c.Name, c.Name);
                                    child.Tag = c;
                                }
                            }
                        }

                        if (this.toolStripStatusLabel1.Text.Length > 0)
                        {
                            Point loc = c.FindForm().PointToClient(c.Parent.PointToScreen(c.Location));
                            int place = this.toolStripStatusLabel1.Text.IndexOf("{X=");
                            string text = this.toolStripStatusLabel1.Text.Substring(0, place);
                            text += loc;
                            this.toolStripStatusLabel1.Text = text;
                        }
                    }
                }
            }

            if (e.Component is LayerWidget)
            {
                //Bless added to fix the wrong location when scrolling after size changed for layer
                LayerWidget layer = GetLayerWidget(this.layerTabControl.SelectedTab);

                if (((layer.Size.Width > 0) && (layer.Size.Height > 0)) && ((ITU.screenWidth != layer.Size.Width) || (ITU.screenHeight != layer.Size.Height)))
                {
                    Control obj = e.Component as Control;
                    obj.Location = new Point(15, 15);

                    ITU.screenWidth = layer.Size.Width;
                    ITU.screenHeight = layer.Size.Height;
                }

            }
        }

        void tp_Enter(object sender, EventArgs e)
        {
            int index = this.layerTabControl.Controls.IndexOf(sender as Control);
            if (HostSurface.hostSurfaces.Count > 0)
            {
                HostSurface hostSurface = HostSurface.hostSurfaces[index];
                ISelectionService ss = hostSurface.GetService(typeof(ISelectionService)) as ISelectionService;
                ss.SetSelectedComponents(null);
                KeystrokMessageFilter.host = hostSurface.formDesignerHost;
                MouseMessageFilter.view = hostSurface.formDesignerHost.RootComponent as Control;
                ITU.layerWidget = hostSurface.formDesignerHost.RootComponent as Control;
                currHostSurface = hostSurface;
            }
        }

        private void AddWidgetItem(Control container, ListBox.ObjectCollection items)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                    items.Add(c.Name);

                AddWidgetItem(c, items);
            }
        }

        private void AddWidgetTree(Control container, TreeNodeCollection nodes, bool inverse)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                {
                    IWidget w = c as IWidget;
                    TreeNode node;

                    if (inverse)
                        node = nodes.Insert(0, c.Name, c.Name);
                    else
                        node = nodes.Add(c.Name, c.Name);

                    if (w.Hided)
                        node.NodeFont = new Font(widgetTreeView.Font, FontStyle.Bold);

                    node.Tag = c;
                    AddWidgetTree(c, node.Nodes, inverse);
                }
                else
                    AddWidgetTree(c, nodes, inverse);
            }
        }

        private void SelectWidgetItem(Control container, object name)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget && c.Name.Equals(name))
                {
                    object[] selection = new object[1];
                    selection[0] = c;

                    HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                    ISelectionService s = (ISelectionService)hostSurface.GetService(typeof(ISelectionService));

                    s.SetSelectedComponents(selection);
                    this.designerPropertyGrid.SelectedObjects = selection;
                    break;
                }
                else
                {
                    SelectWidgetItem(c, name);
                }
            }
        }

        private void InitializeToolbox()
        {
            ToolboxItem item;
            
            item = new System.Drawing.Design.ToolboxItem();
            item.DisplayName = "<Pointer>";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ContainerWidget));
            item.DisplayName = "Container";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(BackgroundWidget));
            item.DisplayName = "Background";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(IconWidget));
            item.DisplayName = "Icon";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TextWidget));
            item.DisplayName = "Text";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TextBoxWidget));
            item.DisplayName = "TextBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ButtonWidget));
            item.DisplayName = "Button";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(CheckBoxWidget));
            item.DisplayName = "CheckBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(RadioBoxWidget));
            item.DisplayName = "RadioBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ScrollTextWidget));
            item.DisplayName = "ScrollText";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ListBoxWidget));
            item.DisplayName = "ListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(FileListBoxWidget));
            item.DisplayName = "FileListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(SpriteWidget));
            item.DisplayName = "Sprite";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ProgressBarWidget));
            item.DisplayName = "ProgressBar";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(KeyboardWidget));
            item.DisplayName = "Keyboard";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(SpriteButtonWidget));
            item.DisplayName = "SpriteButton";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TrackBarWidget));
            item.DisplayName = "TrackBar";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(DigitalClockWidget));
            item.DisplayName = "DigitalClock";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(AnalogClockWidget));
            item.DisplayName = "AnalogClock";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(CalendarWidget));
            item.DisplayName = "Calendar";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(IconListBoxWidget));
            item.DisplayName = "IconListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(MediaFileListBoxWidget));
            item.DisplayName = "MediaFileListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(CircleProgressBarWidget));
            item.DisplayName = "CircleProgressBar";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ScrollBarWidget));
            item.DisplayName = "ScrollBar";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(AnimationWidget));
            item.DisplayName = "Animation";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(WheelWidget));
            item.DisplayName = "Wheel";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(CoverFlowWidget));
            item.DisplayName = "CoverFlow";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(PopupButtonWidget));
            item.DisplayName = "PopupButton";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ScrollListBoxWidget));
            item.DisplayName = "ScrollListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ScrollMediaFileListBoxWidget));
            item.DisplayName = "ScrollMediaFileListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(MeterWidget));
            item.DisplayName = "Meter";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ScrollIconListBoxWidget));
            item.DisplayName = "ScrollIconListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(VideoWidget));
            item.DisplayName = "Video";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ColorPickerWidget));
            item.DisplayName = "ColorPicker";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ImageCoverFlowWidget));
            item.DisplayName = "ImageCoverFlow";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(BackgroundButtonWidget));
            item.DisplayName = "BackgroundButton";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(RippleBackgroundWidget));
            item.DisplayName = "RippleBackground";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(CurveWidget));
            item.DisplayName = "Curve";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TableListBoxWidget));
            item.DisplayName = "TableListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TableIconListBoxWidget));
            item.DisplayName = "TableIconListBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(PopupRadioBoxWidget));
            item.DisplayName = "PopupRadioBox";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(LanguageSpriteWidget));
            item.DisplayName = "LanguageSprite";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(PageFlowWidget));
            item.DisplayName = "PageFlow";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ShadowWidget));
            item.DisplayName = "Shadow";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(AudioWidget));
            item.DisplayName = "Audio";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(SlideshowWidget));
            item.DisplayName = "Slideshow";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(DragIconWidget));
            item.DisplayName = "DragIcon";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(BlurWidget));
            item.DisplayName = "Blur";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ScaleCoverFlowWidget));
            item.DisplayName = "ScaleCoverFlow";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(WheelBackgroundWidget));
            item.DisplayName = "WheelBackground";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(DrawPenWidget));
            item.DisplayName = "DrawPen";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(WaveBackgroundWidget));
            item.DisplayName = "WaveBackground";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(StepWheelWidget));
            item.DisplayName = "StepWheel";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(ClipperWidget));
            item.DisplayName = "Clipper";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(SimpleAnimationWidget));
            item.DisplayName = "SimpleAnimation";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(StopAnywhereWidget));
            item.DisplayName = "StopAnyWhere";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(OscilloscopeWidget));
            item.DisplayName = "Oscilloscope";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TableBarWidget));
            item.DisplayName = "TableBar";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(TableGridWidget));
            item.DisplayName = "TableGrid";
            toolboxListBox.Items.Add(item);

            item = new ToolboxItem(typeof(CustomTextWidget));
            item.DisplayName = "CustomText";
            toolboxListBox.Items.Add(item);

            toolboxListBox.MouseDown += delegate(object sender, MouseEventArgs e)
            {
                if (null == toolboxListBox.SelectedItem)
                    return;

                toolboxListBox.DoDragDrop(toolboxListBox.SelectedItem, DragDropEffects.Copy | DragDropEffects.Move);
            };

            layerListBox.MouseDown += delegate(object sender, MouseEventArgs e)
            {
                if (null == layerListBox.SelectedItem)
                    return;

                foreach (TabPage tab in layerTabControl.TabPages)
                {
                    if (layerListBox.SelectedItem.Equals(tab.Text))
                    {
                        layerTabControl.SelectedTab = tab;

                        widgetListBox.Items.Clear();
                        AddWidgetItem(tab, widgetListBox.Items);

                        widgetTreeView.Nodes.Clear();
                        AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

                        break;
                    }
                }
            };

            this.layerTabControl.layerListBox = layerListBox;

            widgetListBox.MouseDown += delegate(object sender, MouseEventArgs e)
            {
                if (null == widgetListBox.SelectedItem)
                    return;

                SelectWidgetItem(layerTabControl.SelectedTab, widgetListBox.SelectedItem);
            };
            this.layerTabControl.widgetListBox = widgetListBox;

            widgetTreeView.NodeMouseClick += new TreeNodeMouseClickEventHandler(widgetTreeView_NodeMouseClick);
            this.layerTabControl.widgetTreeView = widgetTreeView;

            widgetTreeView.AllowDrop = true;
            widgetTreeView.ItemDrag += new ItemDragEventHandler(widgetTreeView_ItemDrag);
            widgetTreeView.DragEnter += new DragEventHandler(widgetTreeView_DragEnter);
            widgetTreeView.DragDrop += new DragEventHandler(widgetTreeView_DragDrop);
        }

        void widgetTreeView_NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            treeViewSelecting = true;
            SelectWidgetItem(layerTabControl.SelectedTab, e.Node.Name);
            treeViewSelecting = false;
        }

        void widgetTreeView_ItemDrag(object sender, ItemDragEventArgs e)
        {
            DoDragDrop(e.Item, DragDropEffects.Move);
        }

        void widgetTreeView_DragEnter(object sender, DragEventArgs e)
        {
            e.Effect = DragDropEffects.Move;
        }

        void widgetTreeView_DragDrop(object sender, DragEventArgs e)
        {
            // Retrieve the client coordinates of the drop location.
            Point targetPoint = widgetTreeView.PointToClient(new Point(e.X, e.Y));

            // Retrieve the node at the drop location.
            TreeNode targetNode = widgetTreeView.GetNodeAt(targetPoint);

            // Retrieve the node that was dragged.
            TreeNode draggedNode = (TreeNode)e.Data.GetData(typeof(TreeNode));

            // Confirm that the node at the drop location is not 
            // the dragged node and that target node isn't null
            // (for example if you drag outside the control)
            if (!draggedNode.Equals(targetNode) && targetNode != null)
            {
                Control widget = targetNode.Tag as Control;

                if (widget is LayerWidget ||
                    widget is BackgroundWidget ||
                    widget is SpriteWidget ||
                    widget is KeyboardWidget ||
                    widget is SpriteButtonWidget ||
                    widget is TrackBarWidget ||
                    widget is DigitalClockWidget ||
                    widget is AnalogClockWidget ||
                    widget is CalendarWidget ||
                    widget is CircleProgressBarWidget ||
                    widget is AnimationWidget ||
                    widget is CoverFlowWidget ||
                    widget is MeterWidget ||
                    widget is VideoWidget ||
                    widget is ColorPickerWidget ||
                    widget is ImageCoverFlowWidget ||
                    widget is BackgroundButtonWidget ||
                    widget is RippleBackgroundWidget ||
                    widget is CurveWidget ||
                    widget is LanguageSpriteWidget ||
                    widget is PageFlowWidget ||
                    widget is ContainerWidget ||
                    widget is SlideshowWidget ||
                    widget is ScaleCoverFlowWidget ||
                    widget is WheelBackgroundWidget ||
                    widget is WaveBackgroundWidget ||
                    widget is StepWheelWidget ||
                    widget is SimpleAnimationWidget ||
                    widget is StopAnywhereWidget)
                {
                    Control child = draggedNode.Tag as Control;

                    HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                    UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;

                    ues.AddTreeNodeUndoMove(draggedNode.Parent, targetNode, draggedNode, draggedNode.Parent.Nodes.IndexOf(draggedNode));

                    child.Parent.Controls.Remove(child);
                    widget.Controls.Add(child);
                    widget.Controls.SetChildIndex(child, 0);

                    // Remove the node from its current 
                    // location and add it to the node at the drop location.
                    draggedNode.Remove();
                    targetNode.Nodes.Add(draggedNode);

                    // Expand the node at the location 
                    // to show the dropped node.
                    targetNode.Expand();
                }
            }
        }

        private LayerWidget GetLayerWidget(Control container)
        {
            foreach (Control c in container.Controls)
            {
                if (c is LayerWidget)
                    return c as LayerWidget;
                else
                    return GetLayerWidget(c);
            }
            return null;
        }
         
        private void LoadFile(String fileName)
        {
            StreamReader sr = new StreamReader(fileName);
            XmlDocument doc = new XmlDocument();
            
            doc.PreserveWhitespace = true;
            doc.LoadXml(sr.ReadToEnd());
            sr.Close();
            ITU.layerCount = 0;

            XmlNode root = doc.FirstChild;
            for (int i = 0; i < root.ChildNodes.Count; i++)
            {
                if (root.ChildNodes[i] is XmlElement)
                {
                    XmlAttribute typeAttr = root.ChildNodes[i].Attributes["type"];
                    Type type = Type.GetType(typeAttr.Value);
                    if (type == typeof(LayerWidget))
                    {
                        InitializeDesignerSurface(doc, root.ChildNodes[i] as XmlElement);
                        ITU.layerCount = this.layerTabControl.TabCount;
                    }
                }
            }
        }

        private void openToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Save file?", "Confirm save", MessageBoxButtons.YesNo) == DialogResult.Yes)
                saveToolStripMenuItem_Click(sender, e);

            if (GC.GetTotalMemory(false) >= (1024 * 1024 * 300))
            {
                if (MessageBox.Show("Free Memory Low...Restart Now!", "Memory Warning") == DialogResult.OK)
                {
                    System.Diagnostics.Process.Start(Application.ExecutablePath);
                    Environment.Exit(Environment.ExitCode);
                }
            }

            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "xml files (*.xml)|*.xml";

            if (dialog.ShowDialog() == DialogResult.OK && dialog.FileName != "")
            {
                this.xmlPath = dialog.FileName;

                try
                {
                    HostSurface.hostSurfaces.Clear();
                    this.layerTabControl.TabPages.Clear();
                    this.layerListBox.Items.Clear();
                    NameCreationService.Reset();
                    ITU.layerWidget = null;
                    LoadFile(dialog.FileName);

                    foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
                    {
                        UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                        ues.Reset();
                    }

                    StringBuilder sb;
                    int index = dialog.FileName.LastIndexOf('.');
                    if (index == -1)
                        sb = new StringBuilder(dialog.FileName);
                    else
                        sb = new StringBuilder(dialog.FileName, 0, index, 512);

                    sb.Append(".itu");

                    this.ituPath = sb.ToString();
                    ITU.modified = true;
                    this.Text = "GUI Designer - " + dialog.FileName;

                    widgetListBox.Items.Clear();
                    AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

                    widgetTreeView.Nodes.Clear();
                    AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

                    ITU.layerWidget = GetLayerWidget(this.layerTabControl.SelectedTab);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
                }
            }
        }

        private void SaveXml()
        {
            int i, count;
            XmlElement xe;
            XmlNode rootnode;
            XmlNodeList rootlist;
            XmlNode[] zom_list = new XmlNode[10000];

            ITU.Reset();

            XmlDocument document = new XmlDocument();
            document.AppendChild(document.CreateElement("ITU"));

            foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
            {
                hostSurface.widgetLoader.xmlDocument = document;
                hostSurface.widgetLoader.Flush();
            }
            ITU.CalcOffset();
            foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
            {
                hostSurface.widgetLoader.CalcOffset();
            }

            //remove the error zombie root object
            xe = document.DocumentElement;
            rootnode = document.SelectSingleNode("ITU");
            rootlist = rootnode.ChildNodes;
            count = rootlist.Count;

            for (i = 0; i < count; i++)
            {
                XmlElement xx = (XmlElement)rootlist.Item(i);
                XmlAttributeCollection xac = xx.Attributes;
                XmlNode xd = xac.Item(0);

                string str = xd.InnerText;
                if ((!str.Contains("GUIDesigner.LayerWidget")) && (!xd.Value.Contains("AudioWidget")))
                {
                    zom_list[i] = rootlist.Item(i);
                }
                else
                {
                    zom_list[i] = null;
                }
            }

            for (i = 0; i < count; i++)
            {
                if (zom_list[i] != null)
                {
                    rootnode.RemoveChild(zom_list[i]);
                }
            }


            try
            {
                if (this.xmlPath != null)
                {
                    XmlTextWriter xtw = new XmlTextWriter(this.xmlPath, System.Text.Encoding.UTF8);

                    xtw.Formatting = Formatting.Indented;
                    document.WriteTo(xtw);
                    xtw.Close();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: Could not write file to disk. Original error: " + ex.Message);
            }
        }

        private void saveToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.xmlPath == null)
                saveAsToolStripMenuItem_Click(sender, e);
            else
            {
                FileInfo fInfo = new FileInfo(this.xmlPath);
                if (fInfo.Exists)
                {
                    File.Copy(this.xmlPath, this.xmlPath + ".bak", true);
                }
                SaveXml();
            }
        }

        private void saveAsToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "xml files (*.xml)|*.xml";
            dialog.ShowDialog();

            if (dialog.FileName != "")
            {
                this.xmlPath = dialog.FileName;

                FileInfo fInfo = new FileInfo(this.xmlPath);
                if (fInfo.Exists)
                {
                    File.Copy(this.xmlPath, this.xmlPath + ".bak", true);
                }
            }
            else
            {
                if (e is FormClosingEventArgs)
                    ((FormClosingEventArgs)e).Cancel = true;

                return;
            }
            this.Text = "GUI Designer - " + dialog.FileName;

            SaveXml();
        }

        private void exportToolStripMenuItem_Click(object sender, EventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "itu files (*.itu)|*.itu";
            dialog.ShowDialog();

            if (dialog.FileName != "")
            {
                this.ituPath = dialog.FileName;
            }
            else
                return;

            ITU.jpegCompression = jpegCompressionToolStripMenuItem.Checked;
            ITU.jpegAlphaCompression = jpegAlphaCompressionToolStripMenuItem.Checked;
            ITU.bigEndian = bigEndianToolStripMenuItem.Checked;
            ITU.speedyCompression = speedyCompressionToolStripMenuItem.Checked;
            ITU.speedySurfaceCompression = speedySurfaceCompressionToolStripMenuItem.Checked;
            ITU.SaveItu(this.ituPath, compressionToolStripMenuItem.Checked, itu_mode);
        }

        private void exitToolStripMenuItem_Click(object sender, EventArgs e)
        {
            this.Close();
        }

        private DateTime RetrieveLinkerTimestamp()
        {
            string filePath = System.Reflection.Assembly.GetCallingAssembly().Location;
            const int c_PeHeaderOffset = 60;
            const int c_LinkerTimestampOffset = 8;
            byte[] b = new byte[2048];
            System.IO.Stream s = null;

            try
            {
                s = new System.IO.FileStream(filePath, System.IO.FileMode.Open, System.IO.FileAccess.Read);
                s.Read(b, 0, 2048);
            }
            finally
            {
                if (s != null)
                {
                    s.Close();
                }
            }

            int i = System.BitConverter.ToInt32(b, c_PeHeaderOffset);
            int secondsSince1970 = System.BitConverter.ToInt32(b, i + c_LinkerTimestampOffset);
            DateTime dt = new DateTime(1970, 1, 1, 0, 0, 0, DateTimeKind.Utc);
            dt = dt.AddSeconds(secondsSince1970);
            dt = dt.ToLocalTime();
            return dt;
        }

        private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Version version = System.Reflection.Assembly.GetEntryAssembly().GetName().Version;
            DateTime buildDateTime = RetrieveLinkerTimestamp();
            string msgText = "Version: " + version + "\r\nBuild Date: " + buildDateTime;
            string msgText2 = msgText + "\r\n\r\n(When you exit will auto copy these info)";
            //MessageBox.Show("Version: " + version + "\r\nBuild Date: " + buildDateTime, "About " + this.Text);
            if (MessageBox.Show(msgText2, "About " + this.Text) == System.Windows.Forms.DialogResult.OK)
                Clipboard.SetText(msgText);
            else
                Clipboard.SetText(msgText);
        }

        private void undoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
            UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
            ues.Undo();
            ITU.modified = true;
        }

        private void redoToolStripMenuItem_Click(object sender, EventArgs e)
        {
            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
            UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
            ues.Redo();
            ITU.modified = true;
        }

        private void cutToolStripMenuItem_Click(object sender, EventArgs e)
        {
            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
            IMenuCommandService ims = hostSurface.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            bool result = ims.GlobalInvoke(StandardCommands.Cut);
        }

        private void copyToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //MEMORYSTATUS memStatus = new MEMORYSTATUS();
            long m_PhysicalMemory;
            long availablebytes;
            ManagementClass mc = new ManagementClass("Win32_ComputerSystem");
            ManagementClass ms = new ManagementClass("Win32_OperatingSystem");
            ManagementObjectCollection moc = mc.GetInstances();
            ManagementObjectCollection mos = ms.GetInstances();
            foreach (ManagementObject mo in moc)
            {
                if (mo["TotalPhysicalMemory"] != null)
                {
                    m_PhysicalMemory = long.Parse(mo["TotalPhysicalMemory"].ToString());
                }
            }
            foreach (ManagementObject mo in mos)
            {
                if (mo["FreePhysicalMemory"] != null)
                {
                    availablebytes = 1024 * long.Parse(mo["FreePhysicalMemory"].ToString());
                }
            }   

            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
            IMenuCommandService ims = hostSurface.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            ims.GlobalInvoke(StandardCommands.Copy);
        }

        private void pasteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
            IMenuCommandService ims = hostSurface.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            ims.GlobalInvoke(StandardCommands.Paste);
        }

        private void deleteToolStripMenuItem_Click(object sender, EventArgs e)
        {
            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
            IMenuCommandService ims = hostSurface.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            ims.GlobalInvoke(StandardCommands.Delete);
        }

        private void runToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (ITU.screenWidth == 0 || ITU.screenHeight == 0 || this.fontPath == null || this.ituPath == null || bigEndianToolStripMenuItem.Checked)
                return;

            if (ITU.modified || bigEndianToolStripMenuItem.Checked != ITU.bigEndian)
            {
                ITU.jpegCompression = jpegCompressionToolStripMenuItem.Checked;
                ITU.jpegAlphaCompression = jpegAlphaCompressionToolStripMenuItem.Checked;
                ITU.bigEndian = bigEndianToolStripMenuItem.Checked;
                ITU.speedyCompression = speedyCompressionToolStripMenuItem.Checked;
                ITU.speedySurfaceCompression = speedySurfaceCompressionToolStripMenuItem.Checked;
                ITU.SaveItu(this.ituPath, compressionToolStripMenuItem.Checked, itu_mode);
            }

            ProcessStartInfo startInfo = new ProcessStartInfo();
            startInfo.CreateNoWindow = false;
            startInfo.UseShellExecute = false;
            startInfo.FileName = emulatorPath;
            startInfo.WindowStyle = ProcessWindowStyle.Hidden;
            startInfo.Arguments = "" + ITU.screenWidth + " " + ITU.screenHeight + " " + this.fontPath + " " + this.ituPath + " " + (scale50ToolStripMenuItem.Checked ? "50" : "100");

            try
            {
                // Start the process with the info we specified.
                // Call WaitForExit and then the using statement will close.
                using (Process exeProcess = Process.Start(startInfo))
                {
                    exeProcess.WaitForExit();
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: Could not run emulator. Original error: " + ex.Message);
            }
        }

        private void fontPathToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "ttf files (*.ttf)|*.ttf";
            dialog.FileName = this.fontPath;

            if (dialog.ShowDialog() == DialogResult.OK && dialog.FileName != "")
            {
                this.fontPath = dialog.FileName;
                fontPathToolStripMenuItem.Checked = true;
                ITU.fontFace = new Face(ITU.fontLib, fontPath);
            }
        }

        private void newLayerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            InitializeDesignerSurface(null, null);
            ITU.layerCount = this.layerTabControl.TabCount;
        }

        private void removeLayerToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.layerTabControl.TabCount > 1)
            {
                int index = this.layerTabControl.SelectedIndex;
                TabPage tab = this.layerTabControl.TabPages[index];
                this.layerListBox.Items.Remove(tab.Text);
                HostSurface.hostSurfaces.RemoveAt(index);
                this.layerTabControl.TabPages.RemoveAt(index);

                if (index < this.layerTabControl.TabCount)
                    this.layerTabControl.SelectedIndex = index;
                else
                    this.layerTabControl.SelectedIndex = this.layerTabControl.TabCount - 1;

                widgetListBox.Items.Clear();
                AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

                widgetTreeView.Nodes.Clear();
                AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);
            }
            ITU.layerCount = this.layerTabControl.TabCount;
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            DialogResult result = MessageBox.Show("Save file before exit?", "Confirm Exit", MessageBoxButtons.YesNoCancel);
            if (result == DialogResult.Cancel)
            {
                e.Cancel = true;
            }
            else if (result == DialogResult.Yes)
            {
                saveToolStripMenuItem_Click(sender, e);
            }
        }

        private void newToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (MessageBox.Show("Save file?", "Confirm Save", MessageBoxButtons.YesNo) == DialogResult.Yes)
                saveToolStripMenuItem_Click(sender, e);

            HostSurface.hostSurfaces.Clear();
            this.layerTabControl.TabPages.Clear();
            this.layerListBox.Items.Clear();
            NameCreationService.Reset();

            foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
            {
                UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                ues.Reset();
            }
            this.xmlPath = null;
            this.ituPath = null;
            this.imagePath = null;
            this.buildPath = null;
            ITU.Reset();
            this.Text = "GUI Designer";
            InitializeDesignerSurface(null, null);
            ITU.layerCount = this.layerTabControl.TabCount;

            widgetListBox.Items.Clear();
            AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

            widgetTreeView.Nodes.Clear();
            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);
        }

        private void templatelayer(string oldname, string newname)
        {
            foreach (TabPage tab in layerTabControl.TabPages)
            {
                if (tab.Text == oldname)
                {
                    this.layerTabControl.SelectedTab = tab;
                    break;
                }
            }
            this.layerTabControl.TabPages[this.layerTabControl.SelectedIndex].Text = newname;
            NameCreationService.names.Remove(oldname);
            NameCreationService.names.Add(newname);
            this.widgetListBox.Items.Remove(oldname);
            this.widgetListBox.Items.Add(newname);
            TreeNode[] nodes = this.widgetTreeView.Nodes.Find(oldname, true);
            if (nodes.Length > 0)
            {
                nodes[0].Name = newname;
                nodes[0].Text = newname;
            }

            foreach (HostSurface hs in HostSurface.hostSurfaces)
            {
                IWidget widget1 = null;
                bool found = false;
                widget1 = FindWidget(hs.formDesignerHost.Container.Components, oldname);
                if (widget1 != null)
                {
                    IComponent comp = (IComponent)widget1;
                    hs.ComponentContainer.Add(comp, newname);
                    NameCreationService.names.Remove(newname);
                    NameCreationService.names.Remove(newname);
                    found = true;
                }
                if (found)
                {
                    foreach (TabPage tab in layerTabControl.TabPages)
                    {
                        if (tab.Text == newname)
                        {
                            this.layerTabControl.SelectedTab = tab;
                            break;
                        }
                    }
                    widgetListBox.Items.Clear();
                    AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);
                    widgetTreeView.Nodes.Clear();
                    AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);
                    break;
                }
            }
            RefreshTreeView();
        }

        private void templateToolStripMenuItem_Click(object sender, EventArgs e)
        {
            //Working Area
            //same with New function
            if (MessageBox.Show("Save file?", "Confirm Save", MessageBoxButtons.YesNo) == DialogResult.Yes)
                saveToolStripMenuItem_Click(sender, e);

            HostSurface.hostSurfaces.Clear();
            this.layerTabControl.TabPages.Clear();
            this.layerListBox.Items.Clear();
            NameCreationService.Reset();

            foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
            {
                UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                ues.Reset();
            }
            this.xmlPath = null;
            this.ituPath = null;
            this.imagePath = null;
            this.buildPath = null;
            ITU.Reset();
            this.Text = "GUI Designer";
            //logoLayer
            InitializeDesignerSurface(null, null);
            ITU.layerCount = this.layerTabControl.TabCount;
            templatelayer("Layer1", "logoLayer");
            foreach (HostSurface hs in HostSurface.hostSurfaces)
            {
                foreach (IComponent ic in hs.formDesignerHost.Container.Components)
                {
                    IWidget widget = (IWidget)ic;
                    if (widget.Name == "logoLayer")
                    {
                        LayerWidget layer = ic as LayerWidget;
                        LayerWidget.WidgetAction act1 = new LayerWidget.WidgetAction();
                        layer.CFileName = "layer_logo.c";
                        layer.Visibility = true;
                        layer.DoubleBuffered = false;
                        layer.Width = 800;
                        layer.Height = 480;
                        
                        act1.Action = ITU.WidgetActionType.Goto;
                        act1.Event = LayerWidget.WidgetEvent.Enter;
                        act1.Target = "mainLayer";
                        layer.Action01 = act1;

                        foreach (TabPage tab in this.layerTabControl.TabPages)
                        {
                            if (tab.Text == "logoLayer")
                            {
                                BackgroundWidget bw1 = new BackgroundWidget();
                                bw1.Name = "Background1";
                                bw1.Width = 800;
                                bw1.Height = 480;
                                bw1.BackColor = Color.Black;
                                this.layerTabControl.SelectedTab = tab;
                                //NameCreationService.names.Add("Background1");
                                hs.ComponentContainer.Add(bw1, "Background1");
                                layer.Controls.Add(bw1);

                                widgetListBox.Items.Clear();
                                AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);
                                widgetTreeView.Nodes.Clear();
                                AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);
                                RefreshTreeView();
                            }
                        }
                        break;
                    }
                }
            }


            //mainLayer
            InitializeDesignerSurface(null, null);
            ITU.layerCount = this.layerTabControl.TabCount;
            templatelayer("Layer2", "mainLayer");
            foreach (HostSurface hs in HostSurface.hostSurfaces)
            {
                foreach (IComponent ic in hs.formDesignerHost.Container.Components)
                {
                    IWidget widget = (IWidget)ic;
                    if (widget.Name == "mainLayer")
                    {
                        LayerWidget layer = ic as LayerWidget;
                        LayerWidget.WidgetAction act1 = new LayerWidget.WidgetAction();
                        LayerWidget.WidgetAction act2 = new LayerWidget.WidgetAction();
                        LayerWidget.WidgetAction act3 = new LayerWidget.WidgetAction();
                        layer.CFileName = "layer_main.c";
                        layer.Visibility = false;
                        layer.DoubleBuffered = true;
                        layer.Width = 800;
                        layer.Height = 480;
                        act1.Action = ITU.WidgetActionType.Show;
                        act1.Event = LayerWidget.WidgetEvent.Enter;
                        act1.Target = "mainLayer";
                        act2.Action = ITU.WidgetActionType.Function;
                        act2.Event = LayerWidget.WidgetEvent.Enter;
                        act2.Target = "MainOnEnter";
                        act3.Action = ITU.WidgetActionType.Function;
                        act3.Event = LayerWidget.WidgetEvent.Timer;
                        act3.Target = "MainOnTimer";
                        layer.Action01 = act1;
                        layer.Action02 = act2;
                        layer.Action03 = act3;

                        foreach (TabPage tab in this.layerTabControl.TabPages)
                        {
                            if (tab.Text == "mainLayer")
                            {
                                BackgroundButtonWidget bbw1 = new BackgroundButtonWidget();
                                PopupButtonWidget pbw1 = new PopupButtonWidget();
                                PopupButtonWidget.WidgetAction pact1 = new PopupButtonWidget.WidgetAction();
                                string rawstr = "襐乇ഊᨊ\0\r䥈䑒\0Ɛ\0Ĭࠂ\0b핲销\0ų則䈀껎ᳩ\0杁䵁\0놏௼愅\0\t灈女\0ໃ\0ໃǇ澨搀읉䑁呸廭퐁ऀ「산㮼꛹骸Ң⇯켂␈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁃塀蚰而愁ᧂȲ萅搈ைဖ逡Ⱐ䍘䂆낀ౡę숂㊄դࠋ점ᚐℬ⁢篇䍼熉锤\0\0䥅乄깂悂";
                                byte[] bs = System.Text.Encoding.BigEndianUnicode.GetBytes(rawstr);
                                bbw1.Name = "mainBackgroundButton";
                                bbw1.Width = 800;
                                bbw1.Height = 480;
                                pbw1.Name = "PopupButton";
                                pbw1.Width = 400;
                                pbw1.Height = 200;
                                pbw1.Location = new Point(200, 140);
                                pbw1.BackColor = Color.White;
                                pbw1.Text = "Action";
                                pbw1.ForeColor = Color.White;
                                pbw1.Font = new Font("Calibri", 40, pbw1.Font.Style);
                                pact1.Action = ITU.WidgetActionType.Function;
                                pact1.Event = PopupButtonWidget.WidgetEvent.Press;
                                pact1.Target = "MainButtonOnPress";
                                pbw1.Action01 = pact1;

                                MemoryStream ms = new MemoryStream(bs);
                                Bitmap bp = new Bitmap(ms);
                                pbw1.BackgroundImage = (Image)bp;

                                this.layerTabControl.SelectedTab = tab;
                                //NameCreationService.names.Add("mainBackgroundButton");
                                hs.ComponentContainer.Add(bbw1, "mainBackgroundButton");
                                //NameCreationService.names.Add("PopupButton");
                                hs.ComponentContainer.Add(pbw1, "PopupButton");
                                bbw1.Controls.Add(pbw1);
                                layer.Controls.Add(bbw1);
                                

                                widgetListBox.Items.Clear();
                                AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);
                                widgetTreeView.Nodes.Clear();
                                AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);
                                RefreshTreeView();
                            }
                        }
                        break;
                    }
                }
            }

        }

        private void SaveImages(Control container, String path)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                {
                    IWidget widget = c as IWidget;
                    widget.SaveImages(path);
                }
                SaveImages(c, path);
            }
        }

        private void exportImagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (imagePath == null)
            {
                FolderBrowserDialog dialog = new FolderBrowserDialog();

                if (dialog.ShowDialog() == DialogResult.OK)
                {
                    imagePath = dialog.SelectedPath;
                }
                else
                    return;
            }

            ITU.images = new HashSet<string>();

            try
            {
                foreach (TabPage tab in layerTabControl.TabPages)
                {
                    SaveImages(tab, imagePath);
                }
            }
            catch (Exception ex)
            {
                MessageBox.Show("Error: Could not write file to disk. Original error: " + ex.Message);
            }
            ITU.images.Clear();
            ITU.images = null;
        }

        private void importSpriteImagesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (this.designerPropertyGrid.SelectedObject is SpriteWidget)
            {
                SpriteWidget spriteWidget = this.designerPropertyGrid.SelectedObject as SpriteWidget;

                using (OpenFileDialog openFileDialog = new OpenFileDialog())
                {
                    openFileDialog.Filter = "Image files (*.gif;*.jpg;*.jpeg;*.bmp;*.wmf;*.png)|*.gif;*.jpg;*.jpeg;*.bmp;*.wmf;*.png|All files (*.*)|*.*";
                    openFileDialog.Multiselect = true;

                    if (openFileDialog.ShowDialog() == DialogResult.OK)
                    {
                        foreach (String file in openFileDialog.FileNames)
                        {
                            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];

                            IconWidget iconWidget = hostSurface.formDesignerHost.CreateComponent(typeof(IconWidget)) as IconWidget;
                            Image loadedImage = Image.FromFile(file);
                            iconWidget.Image = loadedImage;
                            spriteWidget.Controls.Add(iconWidget);
                            widgetTreeView.Nodes.Clear();
                            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);
                        }
                    }
                }
            }
            else if (this.designerPropertyGrid.SelectedObject is SpriteButtonWidget)
            {
                SpriteButtonWidget spriteWidget = this.designerPropertyGrid.SelectedObject as SpriteButtonWidget;

                using (OpenFileDialog openFileDialog = new OpenFileDialog())
                {
                    openFileDialog.Filter = "Image files (*.gif;*.jpg;*.jpeg;*.bmp;*.wmf;*.png)|*.gif;*.jpg;*.jpeg;*.bmp;*.wmf;*.png|All files (*.*)|*.*";
                    openFileDialog.Multiselect = true;

                    if (openFileDialog.ShowDialog() == DialogResult.OK)
                    {
                        foreach (String file in openFileDialog.FileNames)
                        {
                            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];

                            IconWidget iconWidget = hostSurface.formDesignerHost.CreateComponent(typeof(IconWidget)) as IconWidget;
                            Image loadedImage = Image.FromFile(file);
                            iconWidget.Image = loadedImage;
                            spriteWidget.Controls.Add(iconWidget);
                            widgetTreeView.Nodes.Clear();
                            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);
                        }
                    }
                }
            }
        }

        private void EnableExternal(Control container, bool enable)
        {
            foreach (Control c in container.Controls)
            {
                if (c is BackgroundWidget)
                {
                    BackgroundWidget widget = c as BackgroundWidget;
                    widget.External = enable;
                }
                else if (c is IconWidget)
                {
                    IconWidget widget = c as IconWidget;
                    widget.External = enable;
                }
                else if (c is ButtonWidget)
                {
                    ButtonWidget widget = c as ButtonWidget;
                    widget.External = enable;
                }
                else if (c is CheckBoxWidget)
                {
                    CheckBoxWidget widget = c as CheckBoxWidget;
                    widget.External = enable;
                }
                else if (c is RadioBoxWidget)
                {
                    RadioBoxWidget widget = c as RadioBoxWidget;
                    widget.External = enable;
                }
                else if (c is KeyboardWidget)
                {
                    KeyboardWidget widget = c as KeyboardWidget;
                    widget.External = enable;
                }
                else if (c is TrackBarWidget)
                {
                    TrackBarWidget widget = c as TrackBarWidget;
                    widget.External = enable;
                }
                else if (c is DigitalClockWidget)
                {
                    DigitalClockWidget widget = c as DigitalClockWidget;
                    widget.External = enable;
                }
                else if (c is AnalogClockWidget)
                {
                    AnalogClockWidget widget = c as AnalogClockWidget;
                    widget.External = enable;
                }
                else if (c is CalendarWidget)
                {
                    CalendarWidget widget = c as CalendarWidget;
                    widget.External = enable;
                }
                else if (c is IconListBoxWidget)
                {
                    IconListBoxWidget widget = c as IconListBoxWidget;
                    widget.External = enable;
                }
                else if (c is CircleProgressBarWidget)
                {
                    CircleProgressBarWidget widget = c as CircleProgressBarWidget;
                    widget.External = enable;
                }
                else if (c is PopupButtonWidget)
                {
                    PopupButtonWidget widget = c as PopupButtonWidget;
                    widget.External = enable;
                }
                else if (c is MeterWidget)
                {
                    MeterWidget widget = c as MeterWidget;
                    widget.External = enable;
                }
                else if (c is ScrollIconListBoxWidget)
                {
                    ScrollIconListBoxWidget widget = c as ScrollIconListBoxWidget;
                    widget.External = enable;
                }
                else if (c is ColorPickerWidget)
                {
                    ColorPickerWidget widget = c as ColorPickerWidget;
                    widget.External = enable;
                }
                else if (c is BackgroundButtonWidget)
                {
                    BackgroundButtonWidget widget = c as BackgroundButtonWidget;
                    widget.External = enable;
                }
                else if (c is RippleBackgroundWidget)
                {
                    RippleBackgroundWidget widget = c as RippleBackgroundWidget;
                    widget.External = enable;
                }
                else if (c is CurveWidget)
                {
                    CurveWidget widget = c as CurveWidget;
                    widget.External = enable;
                }
                else if (c is PopupRadioBoxWidget)
                {
                    PopupRadioBoxWidget widget = c as PopupRadioBoxWidget;
                    widget.External = enable;
                }
                else if (c is WheelBackgroundWidget)
                {
                    WheelBackgroundWidget widget = c as WheelBackgroundWidget;
                    widget.External = enable;
                }
                else if (c is WaveBackgroundWidget)
                {
                    WaveBackgroundWidget widget = c as WaveBackgroundWidget;
                    widget.External = enable;
                }
                EnableExternal(c, enable);
            }
        }

        private void enableExternalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            EnableExternal(this.layerTabControl.SelectedTab, true);
            this.designerPropertyGrid.Refresh();
        }

        private void disableExternalToolStripMenuItem_Click(object sender, EventArgs e)
        {
            EnableExternal(this.layerTabControl.SelectedTab, false);
            this.designerPropertyGrid.Refresh();
        }

        private void WriteWidgetNames(Control container, HashSet<string> widgetNames)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                {
                    IWidget widget = c as IWidget;
                    widgetNames.Add(widget.Name);
                }
                WriteWidgetNames(c, widgetNames);
            }
        }

        private void WriteFunctions(Control container, HashSet<string> functions)
        {
            foreach (Control c in container.Controls)
            {
                if (c is IWidget)
                {
                    IWidget widget = c as IWidget;
                    widget.WriteFunctions(functions);
                }
                WriteFunctions(c, functions);
            }
        }

        private void createFunctionTableToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string functionTablePath = null;

            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "c file (*.c)|*.c";
            dialog.InitialDirectory = ITU.projectPath;
            dialog.FileName = "function_table.c";

            if (dialog.ShowDialog() == DialogResult.OK && dialog.FileName != "")
            {
                functionTablePath = dialog.FileName;
            }

            if (functionTablePath != null)
            {
                HashSet<string> functions = new HashSet<string>();

                foreach (TabPage tab in layerTabControl.TabPages)
                {
                    WriteFunctions(tab, functions);
                }
                CodeGenerator generator = new CodeGenerator();
                generator.GenerateFileTable(functionTablePath, functions);
            }
        }

        private void projectDirectoryToolStripMenuItem_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog fbd = new FolderBrowserDialog();

            DialogResult result = fbd.ShowDialog();

            if (!string.IsNullOrWhiteSpace(fbd.SelectedPath))
            {
                ITU.projectPath = fbd.SelectedPath;
            }
        }

        private void createLayerCodeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string layerPath = null;
            LayerWidget layerWidget = GetLayerWidget(this.layerTabControl.SelectedTab);

            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "c file (*.c)|*.c";
            dialog.InitialDirectory = ITU.projectPath;
            if (layerWidget != null)
            {
                dialog.FileName = layerWidget.CFileName;
            }

            if (dialog.ShowDialog() == DialogResult.OK && dialog.FileName != "")
            {
                layerPath = dialog.FileName;
            }

            if (layerPath != null)
            {
                FileInfo fInfo = new FileInfo(layerPath);
                if (fInfo.Exists)
                {
                    File.Copy(layerPath, layerPath + ".bak", true);
                }

                HashSet<string> widgetNames = new HashSet<string>();
                HashSet<string> functions = new HashSet<string>();
                try
                {
                    WriteWidgetNames(this.layerTabControl.SelectedTab, widgetNames);
                    WriteFunctions(this.layerTabControl.SelectedTab, functions);
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: Could not write file to disk. Original error: " + ex.Message);
                }

                CodeGenerator generator = new CodeGenerator();
                generator.GenerateLayerCode(layerPath, widgetNames, functions);
            }
        }

        private void alignTopToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.AlignTop);
        }

        private void alignBottomToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.AlignBottom);
        }

        private void alignLeftToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.AlignLeft);
        }

        private void alignRightToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.AlignRight);
        }

        private void alignHorizontalCentersToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.AlignHorizontalCenters);
        }

        private void alignVerticalCentersToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.AlignVerticalCenters);
        }

        private void centerHorizontallyToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.CenterHorizontally);
        }

        private void centerVerticallyToolStripButton_Click(object sender, EventArgs e)
        {
            IMenuCommandService mcs = KeystrokMessageFilter.host.GetService(typeof(IMenuCommandService)) as IMenuCommandService;
            mcs.GlobalInvoke(MenuCommands.CenterVertically);
        }

        private void showToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is IWidget)
            {
                IWidget widget = treeNode.Tag as IWidget;
                widget.Hided = false;
                this.designerPropertyGrid.Refresh();
                treeNode.Text = widget.Name;
                treeNode.NodeFont = new Font(widgetTreeView.Font, FontStyle.Regular);
            }
        }

        private void hideToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is IWidget)
            {
                IWidget widget = treeNode.Tag as IWidget;
                widget.Hided = true;
                this.designerPropertyGrid.Refresh();
                treeNode.Text = widget.Name;
                treeNode.NodeFont = new Font(widgetTreeView.Font, FontStyle.Bold);
            }
        }

        private void moveUpToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is Control)
            {
                Control c = treeNode.Tag as Control;
                if (c.Parent is IWidget)
                {
                    int index = c.Parent.Controls.GetChildIndex(c);
                    if (index < c.Parent.Controls.Count)
                    {
                        c.Parent.Controls.SetChildIndex(c, index + 1);
                        index = treeNode.Index;

                        TreeNode parentNode = treeNode.Parent;
                        treeNode.Remove();
                        parentNode.Nodes.Insert(index - 1, treeNode);
                        this.widgetTreeView.SelectedNode = treeNode;

                        HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                        UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                        ues.AddTreeNodeUndoMoveUp(this.widgetTreeView, treeNode);
                    }
                }
            }
        }

        private void moveDownToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is Control)
            {
                Control c = treeNode.Tag as Control;
                if (c.Parent is IWidget)
                {
                    int index = c.Parent.Controls.GetChildIndex(c);
                    if (index > 0)
                    {
                        c.Parent.Controls.SetChildIndex(c, index - 1);
                        index = treeNode.Index;

                        TreeNode parentNode = treeNode.Parent;
                        treeNode.Remove();
                        parentNode.Nodes.Insert(index + 1, treeNode);
                        this.widgetTreeView.SelectedNode = treeNode;

                        HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                        UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                        ues.AddTreeNodeUndoMoveDown(this.widgetTreeView, treeNode);
                    }
                }
            }
        }

        private IWidget FindWidget(ComponentCollection components, String name)
        {
            foreach (IComponent comp in components)
            {
                if (comp is IWidget)
                {
                    if ((comp as IWidget).Name.Equals(name) || (comp as IWidget).HasFunctionName(name))
                        return comp as IWidget;
                }
            }
            return null;
        }

        public void SearchWidget(string name, bool all)
        {
            IWidget widget = null;

            if (all)
            {
                foreach (HostSurface hs in HostSurface.hostSurfaces)
                {
                    widget = FindWidget(hs.formDesignerHost.Container.Components, name);
                    if (widget != null)
                        break;
                }
            }
            else if (currHostSurface != null)
            {
                widget = FindWidget(currHostSurface.formDesignerHost.Container.Components, name);
            }

            if (widget != null)
            {
                if (widget is LayerWidget)
                {
                    foreach (TabPage tab in layerTabControl.TabPages)
                    {
                        if (widget.Name.Equals(tab.Text))
                        {
                            layerListBox.SelectedItem = widget.Name;
                            layerTabControl.SelectedTab = tab;

                            widgetListBox.Items.Clear();
                            AddWidgetItem(tab, widgetListBox.Items);

                            widgetTreeView.Nodes.Clear();
                            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

                            object[] selection = new object[1];
                            selection[0] = widget;

                            HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                            ISelectionService s = (ISelectionService)hostSurface.GetService(typeof(ISelectionService));

                            s.SetSelectedComponents(selection);
                            this.designerPropertyGrid.SelectedObjects = selection;

                            break;
                        }
                    }
                }
                else if (widget is Control)
                {
                    Control c = widget as Control;
                    Control parent = c.Parent;
                    while (parent != null && !(parent is LayerWidget))
                    {
                        parent = parent.Parent;
                    }

                    if (parent is LayerWidget)
                    {
                        string layerName = (parent as Control).Name;

                        foreach (TabPage tab in layerTabControl.TabPages)
                        {
                            if (layerName.Equals(tab.Text))
                            {
                                layerListBox.SelectedItem = layerName;
                                layerTabControl.SelectedTab = tab;

                                widgetListBox.Items.Clear();
                                AddWidgetItem(tab, widgetListBox.Items);

                                widgetTreeView.Nodes.Clear();
                                AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

                                object[] selection = new object[1];
                                selection[0] = widget;

                                HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                                ISelectionService s = (ISelectionService)hostSurface.GetService(typeof(ISelectionService));

                                s.SetSelectedComponents(selection);
                                this.designerPropertyGrid.SelectedObjects = selection;

                                break;
                            }
                        }
                    }
                }
                focusedTreeNodeName = widget.Name;
                RefreshTreeView();
            }
        }

        private void findToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            Form searchForm = new SearchForm();
            searchForm.ShowDialog();
        }

        private void duplicateNamesToolStripMenuItem_Click(object sender, EventArgs e)
        {
            DuplicateNamesForm duplicateNamesForm = new DuplicateNamesForm();

            var duplicateKeys = NameCreationService.names.GroupBy(x => x)
                        .Where(group => group.Count() > 1)
                        .Select(group => group.Key);

            foreach (var s in duplicateKeys)
                duplicateNamesForm.AppendLine(s);

            duplicateNamesForm.ShowDialog();
        }

        private void WriteLayerFunctions(Control layer, HashSet<string> functions)
        {
            IWidget widget = layer as IWidget;
            widget.WriteFunctions(functions);

            foreach (Control c in layer.Controls)
            {
                if (c is IWidget)
                {
                    widget = c as IWidget;
                    widget.WriteFunctions(functions);
                }
                WriteFunctions(c, functions);
            }
        }

        private void WriteLayers(Control container, HashSet<string> layers)
        {
            foreach (Control c in container.Controls)
            {
                if (c is LayerWidget)
                {
                    HashSet<string> functions = new HashSet<string>();
                    WriteLayerFunctions(c, functions);

                    if (functions.Count > 0)
                    {
                        LayerWidget layerWidget = c as LayerWidget;
                        layers.Add(layerWidget.CFileName);
                    }
                    return;
                }
                WriteLayers(c, layers);
            }
        }

        private void createLayerCMakeToolStripMenuItem_Click(object sender, EventArgs e)
        {
            string layerCMakePath = null;

            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "cmake file (*.cmake)|*.cmake";
            dialog.InitialDirectory = ITU.projectPath;
            dialog.FileName = "layer.cmake";

            if (dialog.ShowDialog() == DialogResult.OK && dialog.FileName != "")
            {
                layerCMakePath = dialog.FileName;
            }

            if (layerCMakePath != null)
            {
                HashSet<string> layers = new HashSet<string>();

                foreach (TabPage tab in layerTabControl.TabPages)
                {
                    WriteLayers(tab, layers);
                }
                CodeGenerator generator = new CodeGenerator();
                generator.GenerateLayerCMake(layerCMakePath, layers);
            }
        }

        private void moveTopToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is Control)
            {
                Control c = treeNode.Tag as Control;
                if (c.Parent is IWidget)
                {
                    c.Parent.Controls.SetChildIndex(c, c.Parent.Controls.Count - 1);

                    TreeNode parentNode = treeNode.Parent;
                    treeNode.Remove();
                    parentNode.Nodes.Insert(0, treeNode);
                    this.widgetTreeView.SelectedNode = treeNode;

                    HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                    UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                    ues.AddTreeNodeUndoMoveTop(this.widgetTreeView, treeNode);
                }
            }
        }

        private void moveBottomToolStripMenuItem_Click(object sender, EventArgs e)
        {
            TreeNode treeNode = this.widgetTreeView.SelectedNode;
            if (treeNode.Tag is Control)
            {
                Control c = treeNode.Tag as Control;
                if (c.Parent is IWidget)
                {
                    c.Parent.Controls.SetChildIndex(c, 0);

                    TreeNode parentNode = treeNode.Parent;
                    treeNode.Remove();
                    parentNode.Nodes.Add(treeNode);
                    this.widgetTreeView.SelectedNode = treeNode;

                    HostSurface hostSurface = HostSurface.hostSurfaces[this.layerTabControl.SelectedIndex];
                    UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                    ues.AddTreeNodeUndoMoveBottom(this.widgetTreeView, treeNode);
                }
            }
        }

        private void jPEGQualityToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form qualityForm = new JpegQualityForm();
            qualityForm.ShowDialog();
        }

        private void MainForm_Shown(object sender, EventArgs e)
        {
            if (this.WindowState == FormWindowState.Minimized)
                this.Hide();
            else
            {
                if (this.xmlPath != null)
                {
                    string value = this.xmlPath;

                    if (!value.Equals("0"))
                    {
                        this.xmlPath = value;
                        try
                        {
                            HostSurface.hostSurfaces.Clear();
                            this.layerTabControl.TabPages.Clear();
                            this.layerListBox.Items.Clear();
                            NameCreationService.Reset();
                            ITU.layerWidget = null;
                            LoadFile(value);

                            foreach (HostSurface hostSurface in HostSurface.hostSurfaces)
                            {
                                UndoEngineService ues = hostSurface.GetService(typeof(UndoEngine)) as UndoEngineService;
                                ues.Reset();
                            }

                            StringBuilder sb;
                            int index = value.LastIndexOf('.');
                            if (index == -1)
                                sb = new StringBuilder(value);
                            else
                                sb = new StringBuilder(value, 0, index, 512);

                            sb.Append(".itu");

                            this.ituPath = sb.ToString();
                            ITU.modified = true;
                            this.Text = "GUI Designer - " + value;

                            widgetListBox.Items.Clear();
                            AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

                            widgetTreeView.Nodes.Clear();
                            AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, true);

                            ITU.layerWidget = GetLayerWidget(this.layerTabControl.SelectedTab);
                        }
                        catch (Exception ex)
                        {
                            MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
                        }
                    }
                }
            }
        }

        private void buildToolStripMenuItem1_Click(object sender, EventArgs e)
        {
            DialogResult result = MessageBox.Show("Save file before build?", "Confirm Build", MessageBoxButtons.YesNoCancel);
            if (result == DialogResult.Cancel)
            {
                return;
            }
            else if (result == DialogResult.Yes)
            {
                saveToolStripMenuItem_Click(sender, e);
                if (ITU.modified || bigEndianToolStripMenuItem.Checked != ITU.bigEndian)
                {
                    ITU.jpegCompression = jpegCompressionToolStripMenuItem.Checked;
                    ITU.jpegAlphaCompression = jpegAlphaCompressionToolStripMenuItem.Checked;
                    ITU.bigEndian = bigEndianToolStripMenuItem.Checked;
                    ITU.speedyCompression = speedyCompressionToolStripMenuItem.Checked;
                    ITU.speedySurfaceCompression = speedySurfaceCompressionToolStripMenuItem.Checked;
                    ITU.SaveItu(this.ituPath, compressionToolStripMenuItem.Checked, itu_mode);
                }
            }

            if (this.buildPath == null)
            {
                OpenFileDialog dialog = new OpenFileDialog();
                dialog.Filter = "cmd file (*.cmd)|*.cmd";

                if (dialog.ShowDialog() == DialogResult.OK && dialog.FileName != "")
                {
                    this.buildPath = dialog.FileName;
                }
            }

            if (this.buildPath != null)
            {
                ProcessStartInfo startInfo = new ProcessStartInfo();
                startInfo.CreateNoWindow = true;
                startInfo.UseShellExecute = true;
                startInfo.FileName = this.buildPath;
                startInfo.WindowStyle = ProcessWindowStyle.Hidden;
                startInfo.WorkingDirectory = Path.GetDirectoryName(this.buildPath);

                try
                {
                    // Start the process with the info we specified.
                    // Call WaitForExit and then the using statement will close.
                    using (Process exeProcess = Process.Start(startInfo))
                    {
                        exeProcess.WaitForExit();
                    }
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: Could not run build command. Original error: " + ex.Message);
                }
            }
        }

        private void copyActionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (designerPropertyGrid.SelectedGridItem != null)
            {
                string selectstr = designerPropertyGrid.SelectedGridItem.Label;
                string selectparaentstr = designerPropertyGrid.SelectedGridItem.Parent.Label;

                if (((selectstr.Length == 8) && selectstr.Contains("Action"))
                    || ((selectparaentstr.Length == 8) && selectparaentstr.Contains("Action")))
                {
                    int index = 0;
                    GridItem obj = designerPropertyGrid.SelectedGridItem;

                    while (obj.Parent.Label.Contains("Action"))
                    {
                        obj = obj.Parent;
                    }

                    obj = obj.Parent;

                    object_to_copy_action = designerPropertyGrid.SelectedObject.GetType().ToString();

                    foreach (GridItem gi in obj.GridItems)
                    {
                        if (gi.Label.Contains("Action"))
                        {
                            action_copy_str[index] = gi.Value;
                            index++;
                        }
                    }

                    if (index > 0)
                        pasteActionToolStripMenuItem.Enabled = true;
                }
            }
        }

        private void set_obj_value(Type myType, string actstr, object obj, object value)
        {
            System.Reflection.FieldInfo[] myFieldInfos = myType.GetFields(System.Reflection.BindingFlags.Instance | System.Reflection.BindingFlags.Static | System.Reflection.BindingFlags.NonPublic | System.Reflection.BindingFlags.Public);
            foreach (System.Reflection.FieldInfo my in myFieldInfos)
            {
                if (my.Name.Contains(actstr))
                {
                    my.SetValue(obj, value);
                    break;
                }
            }
        }

        private bool set_action_value(object obj, int index, string str1, string str2, string str3, string str4)
        {
            int num = index + 1;
            string actstr;
            Type myType = typeof(GUIDesigner.ITUActionType);
            bool result = false;

            if (num < 10)
                actstr = "Action0" + num.ToString();
            else
                actstr = "Action" + num.ToString();

            if (obj is AnimationWidget)
            {
                myType = typeof(AnimationWidget);
                AnimationWidget.WidgetAction actobj = new AnimationWidget.WidgetAction();
                actobj.Event = (AnimationWidget.WidgetEvent)Enum.Parse(typeof(AnimationWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is BackgroundButtonWidget)
            {
                myType = typeof(BackgroundButtonWidget);
                BackgroundButtonWidget.WidgetAction actobj = new BackgroundButtonWidget.WidgetAction();
                actobj.Event = (BackgroundButtonWidget.WidgetEvent)Enum.Parse(typeof(BackgroundButtonWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ButtonWidget)
            {
                myType = typeof(ButtonWidget);
                ButtonWidget.WidgetAction actobj = new ButtonWidget.WidgetAction();
                actobj.Event = (ButtonWidget.WidgetEvent)Enum.Parse(typeof(ButtonWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is CheckBoxWidget)
            {
                myType = typeof(CheckBoxWidget);
                CheckBoxWidget.WidgetAction actobj = new CheckBoxWidget.WidgetAction();
                actobj.Event = (CheckBoxWidget.WidgetEvent)Enum.Parse(typeof(CheckBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ColorPickerWidget)
            {
                myType = typeof(ColorPickerWidget);
                ColorPickerWidget.WidgetAction actobj = new ColorPickerWidget.WidgetAction();
                actobj.Event = (ColorPickerWidget.WidgetEvent)Enum.Parse(typeof(ColorPickerWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is CoverFlowWidget)
            {
                myType = typeof(CoverFlowWidget);
                CoverFlowWidget.WidgetAction actobj = new CoverFlowWidget.WidgetAction();
                actobj.Event = (CoverFlowWidget.WidgetEvent)Enum.Parse(typeof(CoverFlowWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is DragIconWidget)
            {
                myType = typeof(DragIconWidget);
                DragIconWidget.WidgetAction actobj = new DragIconWidget.WidgetAction();
                actobj.Event = (DragIconWidget.WidgetEvent)Enum.Parse(typeof(DragIconWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is DrawPenWidget)
            {
                myType = typeof(DrawPenWidget);
                DrawPenWidget.WidgetAction actobj = new DrawPenWidget.WidgetAction();
                actobj.Event = (DrawPenWidget.WidgetEvent)Enum.Parse(typeof(DrawPenWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is FileListBoxWidget)
            {
                myType = typeof(FileListBoxWidget);
                FileListBoxWidget.WidgetAction actobj = new FileListBoxWidget.WidgetAction();
                actobj.Event = (FileListBoxWidget.WidgetEvent)Enum.Parse(typeof(FileListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is IconListBoxWidget)
            {
                myType = typeof(IconListBoxWidget);
                IconListBoxWidget.WidgetAction actobj = new IconListBoxWidget.WidgetAction();
                actobj.Event = (IconListBoxWidget.WidgetEvent)Enum.Parse(typeof(IconListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ImageCoverFlowWidget)
            {
                myType = typeof(ImageCoverFlowWidget);
                ImageCoverFlowWidget.WidgetAction actobj = new ImageCoverFlowWidget.WidgetAction();
                actobj.Event = (ImageCoverFlowWidget.WidgetEvent)Enum.Parse(typeof(ImageCoverFlowWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is LayerWidget)
            {
                myType = typeof(LayerWidget);
                LayerWidget.WidgetAction actobj = new LayerWidget.WidgetAction();
                actobj.Event = (LayerWidget.WidgetEvent)Enum.Parse(typeof(LayerWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ListBoxWidget)
            {
                myType = typeof(ListBoxWidget);
                ListBoxWidget.WidgetAction actobj = new ListBoxWidget.WidgetAction();
                actobj.Event = (ListBoxWidget.WidgetEvent)Enum.Parse(typeof(ListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is MediaFileListBoxWidget)
            {
                myType = typeof(MediaFileListBoxWidget);
                MediaFileListBoxWidget.WidgetAction actobj = new MediaFileListBoxWidget.WidgetAction();
                actobj.Event = (MediaFileListBoxWidget.WidgetEvent)Enum.Parse(typeof(MediaFileListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is MeterWidget)
            {
                myType = typeof(MeterWidget);
                MeterWidget.WidgetAction actobj = new MeterWidget.WidgetAction();
                actobj.Event = (MeterWidget.WidgetEvent)Enum.Parse(typeof(MeterWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is PageFlowWidget)
            {
                myType = typeof(PageFlowWidget);
                PageFlowWidget.WidgetAction actobj = new PageFlowWidget.WidgetAction();
                actobj.Event = (PageFlowWidget.WidgetEvent)Enum.Parse(typeof(PageFlowWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is PopupButtonWidget)
            {
                myType = typeof(PopupButtonWidget);
                PopupButtonWidget.WidgetAction actobj = new PopupButtonWidget.WidgetAction();
                actobj.Event = (PopupButtonWidget.WidgetEvent)Enum.Parse(typeof(PopupButtonWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is PopupRadioBoxWidget)
            {
                myType = typeof(PopupRadioBoxWidget);
                PopupRadioBoxWidget.WidgetAction actobj = new PopupRadioBoxWidget.WidgetAction();
                actobj.Event = (PopupRadioBoxWidget.WidgetEvent)Enum.Parse(typeof(PopupRadioBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is RadioBoxWidget)
            {
                myType = typeof(RadioBoxWidget);
                RadioBoxWidget.WidgetAction actobj = new RadioBoxWidget.WidgetAction();
                actobj.Event = (RadioBoxWidget.WidgetEvent)Enum.Parse(typeof(RadioBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ScaleCoverFlowWidget)
            {
                myType = typeof(ScaleCoverFlowWidget);
                ScaleCoverFlowWidget.WidgetAction actobj = new ScaleCoverFlowWidget.WidgetAction();
                actobj.Event = (ScaleCoverFlowWidget.WidgetEvent)Enum.Parse(typeof(ScaleCoverFlowWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ScrollBarWidget)
            {
                myType = typeof(ScrollBarWidget);
                ScrollBarWidget.WidgetAction actobj = new ScrollBarWidget.WidgetAction();
                actobj.Event = (ScrollBarWidget.WidgetEvent)Enum.Parse(typeof(ScrollBarWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ScrollIconListBoxWidget)
            {
                myType = typeof(ScrollIconListBoxWidget);
                ScrollIconListBoxWidget.WidgetAction actobj = new ScrollIconListBoxWidget.WidgetAction();
                actobj.Event = (ScrollIconListBoxWidget.WidgetEvent)Enum.Parse(typeof(ScrollIconListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ScrollListBoxWidget)
            {
                myType = typeof(ScrollListBoxWidget);
                ScrollListBoxWidget.WidgetAction actobj = new ScrollListBoxWidget.WidgetAction();
                actobj.Event = (ScrollListBoxWidget.WidgetEvent)Enum.Parse(typeof(ScrollListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is ScrollMediaFileListBoxWidget)
            {
                myType = typeof(ScrollMediaFileListBoxWidget);
                ScrollMediaFileListBoxWidget.WidgetAction actobj = new ScrollMediaFileListBoxWidget.WidgetAction();
                actobj.Event = (ScrollMediaFileListBoxWidget.WidgetEvent)Enum.Parse(typeof(ScrollMediaFileListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is SlideshowWidget)
            {
                myType = typeof(SlideshowWidget);
                SlideshowWidget.WidgetAction actobj = new SlideshowWidget.WidgetAction();
                actobj.Event = (SlideshowWidget.WidgetEvent)Enum.Parse(typeof(SlideshowWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is SpriteButtonWidget)
            {
                myType = typeof(SpriteButtonWidget);
                SpriteButtonWidget.WidgetAction actobj = new SpriteButtonWidget.WidgetAction();
                actobj.Event = (SpriteButtonWidget.WidgetEvent)Enum.Parse(typeof(SpriteButtonWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is SpriteWidget)
            {
                myType = typeof(SpriteWidget);
                SpriteWidget.WidgetAction actobj = new SpriteWidget.WidgetAction();
                actobj.Event = (SpriteWidget.WidgetEvent)Enum.Parse(typeof(SpriteWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is TableIconListBoxWidget)
            {
                myType = typeof(TableIconListBoxWidget);
                TableIconListBoxWidget.WidgetAction actobj = new TableIconListBoxWidget.WidgetAction();
                actobj.Event = (TableIconListBoxWidget.WidgetEvent)Enum.Parse(typeof(TableIconListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is TableListBoxWidget)
            {
                myType = typeof(TableListBoxWidget);
                TableListBoxWidget.WidgetAction actobj = new TableListBoxWidget.WidgetAction();
                actobj.Event = (TableListBoxWidget.WidgetEvent)Enum.Parse(typeof(TableListBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is TextBoxWidget)
            {
                myType = typeof(TextBoxWidget);
                TextBoxWidget.WidgetAction actobj = new TextBoxWidget.WidgetAction();
                actobj.Event = (TextBoxWidget.WidgetEvent)Enum.Parse(typeof(TextBoxWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is TrackBarWidget)
            {
                myType = typeof(TrackBarWidget);
                TrackBarWidget.WidgetAction actobj = new TrackBarWidget.WidgetAction();
                actobj.Event = (TrackBarWidget.WidgetEvent)Enum.Parse(typeof(TrackBarWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is WheelBackgroundWidget)
            {
                myType = typeof(WheelBackgroundWidget);
                WheelBackgroundWidget.WidgetAction actobj = new WheelBackgroundWidget.WidgetAction();
                actobj.Event = (WheelBackgroundWidget.WidgetEvent)Enum.Parse(typeof(WheelBackgroundWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is WheelWidget)
            {
                myType = typeof(WheelWidget);
                WheelWidget.WidgetAction actobj = new WheelWidget.WidgetAction();
                actobj.Event = (WheelWidget.WidgetEvent)Enum.Parse(typeof(WheelWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is StepWheelWidget)
            {
                myType = typeof(StepWheelWidget);
                StepWheelWidget.WidgetAction actobj = new StepWheelWidget.WidgetAction();
                actobj.Event = (StepWheelWidget.WidgetEvent)Enum.Parse(typeof(StepWheelWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is SimpleAnimationWidget)
            {
                myType = typeof(SimpleAnimationWidget);
                SimpleAnimationWidget.WidgetAction actobj = new SimpleAnimationWidget.WidgetAction();
                actobj.Event = (SimpleAnimationWidget.WidgetEvent)Enum.Parse(typeof(SimpleAnimationWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else if (obj is TableGridWidget)
            {
                myType = typeof(TableGridWidget);
                TableGridWidget.WidgetAction actobj = new TableGridWidget.WidgetAction();
                actobj.Event = (TableGridWidget.WidgetEvent)Enum.Parse(typeof(TableGridWidget.WidgetEvent), str2);
                actobj.Action = (ITU.WidgetActionType)Enum.Parse(typeof(ITU.WidgetActionType), str1);
                actobj.Parameter = str4;
                actobj.Target = str3;
                set_obj_value(myType, actstr, obj, actobj);
                result = true;
            }
            else
                result = false;

            return result;
        }

        private void pasteActionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            if (designerPropertyGrid.SelectedGridItem != null)
            {
                string selectstr = designerPropertyGrid.SelectedGridItem.Label;
                string selectparaentstr = designerPropertyGrid.SelectedGridItem.Parent.Label;

                if (((selectstr.Length == 8) && selectstr.Contains("Action"))
                    || ((selectparaentstr.Length == 8) && selectparaentstr.Contains("Action")))
                {
                    GridItem obj = designerPropertyGrid.SelectedGridItem;

                    while (obj.Parent.Label.Contains("Action"))
                    {
                        obj = obj.Parent;
                    }

                    obj = obj.Parent;

                    if (object_to_copy_action == designerPropertyGrid.SelectedObject.GetType().ToString())
                    {
                        for (int index = 0; index < 15; index++)
                        {
                            string str = action_copy_str[index].ToString();
                            string str1 = str.Split(',')[0];
                            string str2 = str.Split(',')[1];
                            string str3 = str.Split(',')[2];
                            string str4 = str.Split(',')[3];
                            set_action_value(designerPropertyGrid.SelectedObject, index, str1, str2, str3, str4);
                        }
                        designerPropertyGrid.Refresh();
                    }
                }
            }

           // pasteActionToolStripMenuItem.Enabled = false;
        }

        private void changeResolutionToolStripMenuItem_Click(object sender, EventArgs e)
        {
            Form resulotionForm = new ResolutionForm();
            resulotionForm.ShowDialog();
        }

        private void ChangeWidgetResolution(ComponentCollection components, float widthRatio, float heightRatio)
        {
            foreach (IComponent comp in components)
            {
                if (comp is Control)
                {
                    Control c = comp as Control;
                    c.Location = new Point((int)Math.Round(c.Location.X * widthRatio), (int)Math.Round(c.Location.Y * heightRatio));
                    c.Size = new Size((int)Math.Round(c.Size.Width * widthRatio), (int)Math.Round(c.Size.Height * heightRatio));

                    if (c is Label)
                    {
                        Label l = c as Label;
                        l.Font = new Font(l.Font.FontFamily, l.Font.Size * heightRatio, l.Font.Style);
                    }
                    else if (c is ButtonBase)
                    {
                        ButtonBase b = c as ButtonBase;
                        b.Font = new Font(b.Font.FontFamily, b.Font.Size * heightRatio, b.Font.Style);
                    }
                    else if (c is ListBox)
                    {
                        ListBox l = c as ListBox;
                        l.Font = new Font(l.Font.FontFamily, l.Font.Size * heightRatio, l.Font.Style);
                    }
                    else if (c is BackgroundWidget)
                    {
                        BackgroundWidget b = c as BackgroundWidget;
                        b.OriginalSize = new Size((int)Math.Round(b.OriginalSize.Width * widthRatio), (int)Math.Round(b.OriginalSize.Height * heightRatio));
                    }
                }
            }
        }

        public void ChangeResolution(int width, int height)
        {
            if (width == 0 || height == 0)
                return;

            if (ITU.screenWidth == width && ITU.screenHeight == height)
                return;

            float widthRatio = (float)width / ITU.screenWidth;
            float heightRatio = (float)height / ITU.screenHeight;

            foreach (HostSurface hs in HostSurface.hostSurfaces)
            {
                ChangeWidgetResolution(hs.formDesignerHost.Container.Components, widthRatio, heightRatio);
            }

            ITU.screenWidth = width;
            ITU.screenHeight = height;
        }

        private void ImportFile(String fileName)
        {
            StreamReader sr = new StreamReader(fileName);
            XmlDocument doc = new XmlDocument();

            doc.PreserveWhitespace = true;
            doc.LoadXml(sr.ReadToEnd());
            sr.Close();

            XmlNode root = doc.FirstChild;
            for (int i = 0; i < root.ChildNodes.Count; i++)
            {
                if (root.ChildNodes[i] is XmlElement)
                {
                    XmlAttribute typeAttr = root.ChildNodes[i].Attributes["type"];
                    Type type = Type.GetType(typeAttr.Value);
                    if (type == typeof(LayerWidget))
                    {
                        InitializeDesignerSurface(doc, root.ChildNodes[i] as XmlElement);
                        ITU.layerCount = this.layerTabControl.TabCount;
                    }
                }
            }
        }

        private void importToolStripMenuItem_Click(object sender, EventArgs e)
        {
            OpenFileDialog dialog = new OpenFileDialog();
            dialog.Filter = "XML files (*.xml)|*.xml";
            dialog.Multiselect = true;

            if (dialog.ShowDialog() == DialogResult.OK)
            {
                try
                {
                    foreach (String file in dialog.FileNames)
                    {
                        ImportFile(file);
                    }
                    ITU.modified = true;
                    widgetListBox.Items.Clear();
                    AddWidgetItem(this.layerTabControl.SelectedTab, widgetListBox.Items);

                    widgetTreeView.Nodes.Clear();
                    AddWidgetTree(this.layerTabControl.SelectedTab, widgetTreeView.Nodes, false);

                    ITU.layerWidget = GetLayerWidget(this.layerTabControl.SelectedTab);
                    ITU.layerCount = this.layerTabControl.TabCount;
                }
                catch (Exception ex)
                {
                    MessageBox.Show("Error: Could not read file from disk. Original error: " + ex.Message);
                }
            }
        }

        private void showWidgetToolStripMenuItem_Click(object sender, EventArgs e)
        {
            showToolStripMenuItem_Click(sender, e);
        }

        private void hideWidgettoolStripMenuItem_Click(object sender, EventArgs e)
        {
            hideToolStripMenuItem_Click(sender, e);
        }

        private void designerPropertyGrid_SelectedGridItemChanged(object sender, SelectedGridItemChangedEventArgs e)
        {
            if (designerPropertyGrid.SelectedGridItem != null)
            {
                GridItem obj = designerPropertyGrid.SelectedGridItem;
                if (obj.Value is Image)
                {

                    showtmnPictureBox.Image = (Image)obj.Value;
                }
                else
                {
                    showtmnPictureBox.Image = null;
                }
            }
        }
    }
}
