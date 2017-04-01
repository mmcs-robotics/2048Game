using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Lego.Ev3.Core;
using Lego.Ev3.Desktop;
using System.Windows.Forms;
using System.Drawing;
using System.Diagnostics;

namespace AForge.WindowsForms
{
    class LegoRobot
    {
        public enum mtr { MotorA, MotorB };

        public Brick _brick = null;  //  «Кирпичик» – контроллер управления

        private bool AMotorReady = true;  //  Готов ли к работе мотор А
        private bool BMotorReady = true;  //  Готов ли к работе мотор В
        private Timer MotorActivityTimer = null;  //  Таймер, отсекающий время для выполнения вращения
        private int tmrInterval = 4000;  //  Интервал неактивности для мотора, 4 сек по умолчанию

        private int angleA;  //  Угол поворота оси мотора А
        private int angleB;  //  Угол поворота оси мотора B

        private PictureBox odometer = null;  //  Картинка, показывающая текущую позицию моторов
        //private Label modeLabel = null;  //  Ну как бы непонятная фигня, указывающая режим мотора А, в виде метки

        delegate void UpdatePictureBox();

        /// <summary>
        /// Определяет угол для поворота. Если CCW - брать скорость с плюсом, иначе с минусом
        /// </summary>
        /// <param name="angle"></param>
        /// <param name="CW"></param>
        /// <returns></returns>
        static uint rotate(int angle, bool CW)
        {
            int result = 0;

            if (angle > 0)
                if (CW)
                    result = 540 - (angle + 180) % 360;
                else
                    result = 180 + (angle + 180) % 360;
            else
                if (CW)
                result = 180 - (angle - 180) % 360;
            else
                result = 540 + (angle - 180) % 360;
            return Convert.ToUInt32(result);
        }

        public LegoRobot()
        {
            MotorActivityTimer = new Timer();
            MotorActivityTimer.Interval = tmrInterval;
            MotorActivityTimer.Tick += new EventHandler(TimerEvent);
        }

        public void SetOdometer(PictureBox pb)
        {
            odometer = pb;
        }

        public void Update()
        {
            if (odometer == null) return;
            if (odometer.InvokeRequired)
            {
                odometer.Invoke(new UpdatePictureBox(Update));
                return;
            }

            if (odometer.Image == null)
                odometer.Image = new Bitmap(odometer.Width, odometer.Height); ;

            Graphics g = Graphics.FromImage(odometer.Image);
            g.Clear(System.Drawing.Color.Transparent);

            int w = odometer.Width - 1;
            int h = odometer.Height - 1;
            int radius = w > h ? h / 2 - 1 : w / 2 - 1;

            Pen p = new Pen(System.Drawing.Color.LightGray, 2);

            g.DrawEllipse(p, new Rectangle(0, 0, w, h));

            int coordX = radius - Convert.ToInt32(System.Math.Round(0.95 * radius * System.Math.Sin(angleA * System.Math.PI / 180)));
            int coordY = radius - Convert.ToInt32(System.Math.Round(0.95 * radius * System.Math.Cos(angleA * System.Math.PI / 180)));
            p.Color = System.Drawing.Color.Green;
            g.DrawLine(p, radius, radius, coordX, coordY);

            coordX = radius - Convert.ToInt32(System.Math.Round(0.95 * radius * System.Math.Sin(angleB * System.Math.PI / 180)));
            coordY = radius - Convert.ToInt32(System.Math.Round(0.95 * radius * System.Math.Cos(angleB * System.Math.PI / 180)));
            p.Color = System.Drawing.Color.Blue;
            g.DrawLine(p, radius, radius, coordX, coordY);

            Font drawFont = new Font("Arial", 11);
            SolidBrush drawBrush = new SolidBrush(System.Drawing.Color.Black);

            g.DrawString("A:" + angleA.ToString(), drawFont, drawBrush, radius - 20, radius + 5);
            g.DrawString("B:" + angleB.ToString(), drawFont, drawBrush, radius - 20, radius + 30);
            odometer.Invalidate();
        }


        /// <summary>
        /// Изменение интервала таймера
        /// </summary>
        /// <param name="newInterval">Новый интервал неактивности мотора в миллисекундах</param>
        public void setTimerInterval(int newInterval)
        {
            if (newInterval > 0)
                MotorActivityTimer.Interval = newInterval;
        }

        /// <summary>
        /// Готовность робота к следующей команде
        /// </summary>
        /// <returns></returns>
        public bool Ready()
        {
            return AMotorReady && BMotorReady;
        }

        /// <summary>
        /// Соединение с роботом по BT
        /// </summary>
        public async void Connect(string ComPort)
        {
            if (_brick != null) return;
            //_brick = new Brick(new UsbCommunication());
            _brick = new Brick(new BluetoothCommunication(ComPort));
            _brick.BrickChanged += _brick_BrickChanged;
            await _brick.ConnectAsync();
            return;
        }

        private void _brick_BrickChanged(object sender, BrickChangedEventArgs e)
        {
            angleA = e.Ports[InputPort.A].RawValue;
            angleB = e.Ports[InputPort.B].RawValue;
            Update();
        }

        public void Disconnect()
        {
            if (_brick != null)
                _brick.Disconnect();
            _brick = null;
        }

        /// <summary>
        /// Обработчик события таймера - переключает цвет блока и делает активными моторы
        /// </summary>
        /// <param name="myObject"></param>
        /// <param name="myEventArgs"></param>
        private void TimerEvent(Object myObject, EventArgs myEventArgs)
        {
            MotorActivityTimer.Stop();
            AMotorReady = true;
            BMotorReady = true;
            if (_brick != null)
                _brick.DirectCommand.SetLedPatternAsync(LedPattern.Green);
        }

        public async void RotateRight(mtr Motor, int speed = 25)
        {
            if (_brick == null) return;
            //int angle = 0;
            if (Motor == mtr.MotorA)
            {
                if (AMotorReady == false) return;
                AMotorReady = false;
            }
            else
            {
                if (BMotorReady == false) return;
                BMotorReady = false;
            }
            int angle = angleA;
            if (Motor == mtr.MotorB) angle = angleB;

            Debug.WriteLine("Угол перед поворотом : " + angle.ToString());
            uint rotateAngle = rotate(angle, true);
            Debug.WriteLine("Угол поворота : " + rotateAngle.ToString() + "; Скорость : " + speed.ToString());
            Debug.WriteLine("Ожидаемый угол : " + (angle + rotateAngle).ToString());

            await _brick.DirectCommand.SetLedPatternAsync(LedPattern.Red);
            //  Запускаем таймер 
            MotorActivityTimer.Start();

            //  Выполняем команду
            if (Motor == mtr.MotorA)
                await _brick.DirectCommand.StepMotorAtSpeedAsync(OutputPort.A, speed, rotateAngle, true);
            else
                await _brick.DirectCommand.StepMotorAtSpeedAsync(OutputPort.B, speed, rotateAngle, true);
        }

        public async void RotateLeft(mtr Motor, int speed = 25)
        {
            if (_brick == null) return;
            //int angle = 0;
            if (Motor == mtr.MotorA)
            {
                if (AMotorReady == false) return;
                AMotorReady = false;
            }
            else
            {
                if (BMotorReady == false) return;
                BMotorReady = false;
            }
            int angle = angleA;
            if (Motor == mtr.MotorB) angle = angleB;

            Debug.WriteLine("Угол перед поворотом : " + angle.ToString());
            uint rotateAngle = rotate(angle, true);
            Debug.WriteLine("Угол поворота : " + rotateAngle.ToString() + "; Скорость : " + speed.ToString());
            Debug.WriteLine("Ожидаемый угол : " + (angle - rotateAngle).ToString());
            await _brick.DirectCommand.SetLedPatternAsync(LedPattern.GreenFlash);
            //  Запускаем таймер 
            MotorActivityTimer.Start();

            //  Выполняем команду
            if (Motor == mtr.MotorA)
                await _brick.DirectCommand.StepMotorAtSpeedAsync(OutputPort.A, -speed, rotateAngle, true);
            else
                await _brick.DirectCommand.StepMotorAtSpeedAsync(OutputPort.B, -speed, rotateAngle, true);
        }

        public async void reset()
        {
            //  Сброс моторов. Однократно выполнить, выставить моторы, и ещё раз выполнить сброс
            if (_brick == null) return;
            await _brick.DirectCommand.StopMotorAsync(OutputPort.A, false);
            await _brick.DirectCommand.StopMotorAsync(OutputPort.B, false);
            await _brick.DirectCommand.ClearChanges(InputPort.A);
            await _brick.DirectCommand.ClearChanges(InputPort.B);
            await _brick.DirectCommand.ClearAllDevices();
        }
    }
}
