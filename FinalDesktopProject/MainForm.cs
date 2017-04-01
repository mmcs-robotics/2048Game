using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using AForge.Video;
using AForge.Video.DirectShow;
using System.Diagnostics;

namespace AForge.WindowsForms
{
    public partial class MainForm : Form
    {
        /// <summary>
        /// Возможные состояния - ожидание кадра, распознавание, анализ, движение.
        /// Их бы в какой-нибудь класс-диспетчер засунуть, но пока так
        /// </summary>
        enum Stage { Idle, WaitingForFrame, Recognition, Thinking, Moving };

        /// <summary>
        /// Играет ли робот. Если true, то при подключенном роботе выполняется игра
        /// </summary>
        bool RobotPlaying = false;

        /// <summary>
        /// Событие для синхронизации таймера
        /// </summary>
        private AutoResetEvent evnt = new AutoResetEvent(false);
        
        /// <summary>
        /// Текущее состояние
        /// </summary>
        private Stage currentState = Stage.Idle;
        
        /// <summary>
        /// Список устройств для снятия видео (веб-камер)
        /// </summary>
        private FilterInfoCollection videoDevicesList;
        
        /// <summary>
        /// Выбранное устройство для видео
        /// </summary>
        private IVideoSource videoSource;
        
        /// <summary>
        /// Анализатор изображения - выполняет преобразования изображения с камеры и сопоставление с шаблонами
        /// </summary>
        private MagicEye processor = new MagicEye();
        
        /// <summary>
        /// Класс для нахождения очередного хода. Получает позицию, выдаёт направление слайда
        /// </summary>
        private Solver sage = new Solver();

        /// <summary>
        /// Робот и весь его внутренний мир
        /// </summary>
        private LegoRobot rbt;

        /// <summary>
        /// Массив картинок на правой панели
        /// </summary>
        private PictureBox[,] pics;
        
        /// <summary>
        /// Таймер для измерения производительности (времени на обработку кадра)
        /// </summary>
        private Stopwatch sw = new Stopwatch();
        
        /// <summary>
        /// Таймер для обновления объектов интерфейса
        /// </summary>
        System.Threading.Timer updateTmr;

        delegate void MyUpdate();
        
        /// <summary>
        /// Функция обновления формы, тут же происходит анализ текущего этапа, и при необходимости переключение на следующий
        /// </summary>
        private void UpdateFormFields()
        {
            
            if (label7.InvokeRequired)
            {
                label7.Invoke(new MyUpdate(UpdateFormFields));
                return;
            }

            label10.Text = "Тики : " + sw.Elapsed.ToString();
            
            //  Выводим в правую панель фрагменты изображений. Они должны содержать только цифры в чёрно-белом формате
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    if(processor.arrayPics[r, c]!=null)
                        pics[r, c].Image = processor.finalPics[r, c].ToManagedImage();

            //  Если статус ожидание, то просто выходим - меняется он не тут
            if (currentState == Stage.Idle)
            {
                label7.Text = "Статус: ожидание";
                return;
            }

            //  Если происходит анализ позиции и поиск хода
            if (currentState==Stage.Thinking)
            {
                label7.Text = "Статус: поиск хода";
                if (sage.workDone())
                {
                    //  Позиция обработана, ход найден
                    currentState = Stage.Moving;

                    string s = "Сдвиг : ";
                    switch (sage.suggestedMove)
                    {
                        case 0: s = "Ничего не понял!"; break;
                        case 1: s = " Вверх!"; break;
                        case 2: s = " Вправо!"; break;
                        case 3: s = " Вниз!"; break;
                        case 4: s = " Влево!"; break;
                    }
                    label6.Text = s;
                    
                    //  Если робот готов к работе, то делаем ход, иначе ничего не меняется - состояние остаётся таким же
                    //    это плохо, строго говоря - строчки эти постоянно переписывать в форму, ну да ладно
                    if(RobotPlaying && rbt!=null && rbt.Ready())
                    {
                        int speed = int.Parse(speedBox.Text);
                        currentState = Stage.Moving;
                        switch (sage.suggestedMove)
                        {
                            case 1: rbt.RotateLeft(LegoRobot.mtr.MotorA, speed); break;
                            case 2: rbt.RotateLeft(LegoRobot.mtr.MotorB, speed); break;
                            case 3: rbt.RotateRight(LegoRobot.mtr.MotorA, speed); break;
                            case 4: rbt.RotateRight(LegoRobot.mtr.MotorB, speed); break;
                        }
                    }

                }
                return;
            }

            if(currentState==Stage.Moving)
            {
                label7.Text = "Статус: выполняю ход";
                if (rbt.Ready())
                    currentState = Stage.WaitingForFrame;
                return;
            }
        }

        /// <summary>
        /// Обёртка для обновления формы - перерисовки картинок, изменения состояния и прочего
        /// </summary>
        /// <param name="StateInfo"></param>
        public void Tick(object StateInfo)
        {
            UpdateFormFields();
            return;
        }

        public MainForm()
        {
            InitializeComponent();
            // Список камер получаем
            videoDevicesList = new FilterInfoCollection(FilterCategory.VideoInputDevice);
            foreach (FilterInfo videoDevice in videoDevicesList)
            {
                cmbVideoSource.Items.Add(videoDevice.Name);
            }
            if (cmbVideoSource.Items.Count > 0)
            {
                cmbVideoSource.SelectedIndex = 0;
            }
            else
            {
                MessageBox.Show("А нет у вас камеры!", "Ошибочка", MessageBoxButtons.OK, MessageBoxIcon.Error);
            }
            // Закрыть по завершению
            this.Closing += Form1_Closing;

            pics = new PictureBox[4, 4];
            for(int r=0;r<4;++r)
                for(int c=0;c<4;++c)
                    {
                        pics[r, c] = new PictureBox();
                        pics[r, c].Width = 100;
                        pics[r, c].Height = 100;
                        pics[r, c].Left = 5 + c * 110;
                        pics[r, c].Top = 5 + r * 110;
                        panel1.Controls.Add(pics[r, c]);
                    }
            rbt = new LegoRobot();
            
            updateTmr = new System.Threading.Timer(Tick, evnt, 500, 100);
            rbt.SetOdometer(pictureBox2);
        }

        private void Form1_Closing(object sender, CancelEventArgs e)
        {
            updateTmr.Dispose();
            updateTmr = null;

            //  Как-то надо ещё робота подождать, если он работает

            if (videoSource != null && videoSource.IsRunning)
            {
                videoSource.SignalToStop();
            }

            //  Тут ошибка вылетает! Надо дождаться все ассоциированные процессы

        }

        private void video_NewFrame(object sender,NewFrameEventArgs eventArgs)
        {
            //  Время засекаем
            sw.Restart();
            //sw.Start();

            //  Отправляем изображение на обработку, и выводим оригинал (с раскраской) и разрезанные изображения

            bool justShowFrame = true;
            if(currentState == Stage.WaitingForFrame)
            {
                currentState = Stage.Recognition;
                justShowFrame = false;
            }


            processor.ProcessImage((Bitmap)eventArgs.Frame.Clone(), justShowFrame);

            pictureBox1.Image = processor.original;

            sw.Stop();

            if (justShowFrame) return;

            currentState = Stage.Thinking;
            sage.solveState(processor.currentDeskState, 16, 7);

        }

        private void btnStart_Click(object sender, EventArgs e)
        {
            if (videoSource == null)
            {
                videoSource = new VideoCaptureDevice(videoDevicesList[cmbVideoSource.SelectedIndex].MonikerString);
                videoSource.NewFrame += new NewFrameEventHandler(video_NewFrame);
                videoSource.Start();
                btnStart.Text = "Стоп";
                controlPanel.Enabled = true;
            }
            else
            {
                videoSource.SignalToStop();
                if (videoSource != null && videoSource.IsRunning && pictureBox1.Image != null)
                {
                    pictureBox1.Image.Dispose();
                }
                videoSource = null;
                btnStart.Text = "Старт";
                controlPanel.Enabled = false;
            }
        }

        private void btnStop_Click(object sender, EventArgs e)
        {
        }

        private void trackBar1_ValueChanged(object sender, EventArgs e)
        {
            processor.settings.threshold = (byte)trackBar1.Value;
            processor.settings.differenceLim = (float)trackBar1.Value/trackBar1.Maximum;
        }

        private void borderTrackBar_ValueChanged(object sender, EventArgs e)
        {
            processor.settings.border = borderTrackBar.Value;
        }

        private void marginTrackBar_ValueChanged(object sender, EventArgs e)
        {
            processor.settings.margin = marginTrackBar.Value;
        }

        private void button1_Click(object sender, EventArgs e)
        {
            currentState = Stage.WaitingForFrame;
        }

        private void button2_Click(object sender, EventArgs e)
        {
            if(RobotPlaying == true)
            {
                //  Выключаем игру
                RobotPlaying = false;
                button2.Text = "Играть";
                currentState = Stage.WaitingForFrame;
            }
            else
            {
                //  Выключаем игру
                RobotPlaying = true;
                button2.Text = "Стоп-кран";
                currentState = Stage.WaitingForFrame;
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            currentState = Stage.WaitingForFrame;
        }

        private void button4_Click(object sender, EventArgs e)
        {
            rbt.RotateLeft(LegoRobot.mtr.MotorA, int.Parse(speedBox.Text));
        }

        private void button5_Click(object sender, EventArgs e)
        {
            rbt.RotateRight(LegoRobot.mtr.MotorA, int.Parse(speedBox.Text));
        }

        private void MainForm_FormClosing(object sender, FormClosingEventArgs e)
        {
            rbt.Disconnect();
            if (updateTmr != null)
            {
                updateTmr.Change(Timeout.Infinite, Timeout.Infinite);
            }
        }

        private void button5_Click_1(object sender, EventArgs e)
        {
            rbt.RotateLeft(LegoRobot.mtr.MotorA, int.Parse(speedBox.Text));
        }

        private void button4_Click_1(object sender, EventArgs e)
        {
            rbt.RotateRight(LegoRobot.mtr.MotorA, int.Parse(speedBox.Text));
        }

        private void btnRight_Click(object sender, EventArgs e)
        {
            rbt.RotateLeft(LegoRobot.mtr.MotorB, int.Parse(speedBox.Text));
        }

        private void btnLeft_Click(object sender, EventArgs e)
        {
            rbt.RotateRight(LegoRobot.mtr.MotorB, int.Parse(speedBox.Text));
        }

        private void button7_Click(object sender, EventArgs e)
        {
            rbt.reset();
        }

        private void btnDisconnect_Click(object sender, EventArgs e)
        {
            rbt.Disconnect();
        }

        private void btnConnect_Click(object sender, EventArgs e)
        {
            rbt.Connect(comPortsNames.Text);
        }

        private void speedBox_TextChanged(object sender, EventArgs e)
        {
            int newInterval;
            //  Если в поле speedBox не целое число, то выход
            if (!int.TryParse(speedBox.Text, out newInterval)) return;
            //  Если указана пауза моторов меньше 50 мс или больше 7 секунд - выход, это некорректные значения
            if (newInterval < 50 || newInterval > 7000) return;
            //  Устанавливаем интервал неактивности робота
            rbt.setTimerInterval(newInterval);
        }
    }
}
