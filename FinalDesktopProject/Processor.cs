using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Drawing;
using System.Diagnostics;

namespace AForge.WindowsForms
{
    class Settings
    {
        private int _border = 20;
        public int border
        {
            get
            {
                return _border;
            }
            set
            {
                if ((value > 0) && (value < height / 3))
                {
                    _border = value;
                    if (top > 2 * _border) top = 2 * _border;
                    if (left > 2 * _border) left = 2 * _border;
                }
            }
        }

        public int width = 640;
        public int height = 640;


        public int margin = 10;
        public int top = 40;
        public int left = 40;

        /// <summary>
        /// Порог при отсечении по цвету 
        /// </summary>
        public byte threshold = 120;
        public float differenceLim = 0.15f;

        public void incTop() { if (top < 2 * _border) ++top; }
        public void decTop() { if (top > 0) --top; }
        public void incLeft() { if (left < 2 * _border) ++left; }
        public void decLeft() { if (left > 0) --left; }
    }

    class MagicEye
    {
        /// <summary>
        /// Массив изображений, получаемых из кадра
        /// </summary>
        public AForge.Imaging.UnmanagedImage[,] arrayPics = new AForge.Imaging.UnmanagedImage[4,4];
        
        /// <summary>
        /// Массив обработанных изображений
        /// </summary>
        public AForge.Imaging.UnmanagedImage[,] finalPics = new AForge.Imaging.UnmanagedImage[4, 4];

        /// <summary>
        /// Оригинальное изображение и обработанное
        /// </summary>
        public AForge.Imaging.UnmanagedImage processed;
        public Bitmap original;

        /// <summary>
        ///  Массив шаблонных изображений для распознавания. Образ и его значение
        /// </summary>
        Dictionary<int, AForge.Imaging.UnmanagedImage> samples;
        
        /// <summary>
        /// Класс настроек
        /// </summary>
        public Settings settings = new Settings();

        public Dictionary<int, int> powers = new Dictionary<int, int>();
        
        /// <summary>
        /// Текущее состояние поля
        /// </summary>
        public byte[] currentDeskState = new byte[16];
        
        /// <summary>
        /// Состояние поля на предыдущем шаге
        /// </summary>
        public byte[] expectedDeskState = null;

        public int errorCount = 0;
        public bool stopByErrors = false;

        /// <summary>
        /// Здесь надо инициализировать изображения
        /// </summary>
        public MagicEye()
        {
            //  Инициализация изображений - предобработанных шаблонов
            powers[0] = 0;
            int pwr = 1;
            for (int i=0;i<17;++i)
            {
                powers[pwr] = i;
                pwr *= 2;
            }
            samples = new Dictionary<int, AForge.Imaging.UnmanagedImage>();

            AForge.Imaging.Filters.Grayscale GSfilter = new AForge.Imaging.Filters.Grayscale(0.2125, 0.7154, 0.0721);

            //  Загружаем образцы чисел
            for (int i = 1; i < 14; ++i)
            {
                string fname = "Images/" + i.ToString() + ".jpg";
                AForge.Imaging.UnmanagedImage tmp = AForge.Imaging.UnmanagedImage.FromManagedImage(new Bitmap(fname));
                samples.Add(i, tmp);
            }
        }

        /// <summary>
        /// Установить изображение в данной клетке как шаблон без распознавания
        /// </summary>
        public void setNewTemplate(int r, int c, int value)
        {
            samples[powers[value]] = (AForge.Imaging.UnmanagedImage)arrayPics[r, c].Clone();
        }
        
        /// <summary>
        /// Запомнить ожидаемое после хода состояние
        /// </summary>
        /// <param name="buffer"></param>
        public void setExpectedState(byte[] buffer)
        {
            expectedDeskState = buffer;
        }
        
        /// <summary>
        /// Исправление текущего состояния на основе предсказанного
        /// </summary>
        private void correctState()
        {
            int iterationErrorCount = 0;
            int lethalErrors = 0;

            if (expectedDeskState == null) return;
            //  Считаем количество ошибок на основе предсказанного состояния
            for (int i = 0; i < 16; ++i)
            {
                if (currentDeskState[i] != expectedDeskState[i])
                    iterationErrorCount++;
            }
            
            //  Если ошибок больше двух - что-то пошло не так
            if (iterationErrorCount > 3)
            {
                Debug.WriteLine("Too many errors on one iteration : " + iterationErrorCount.ToString());
                return;
            }

            //  Пытаемся исправить ошибки распознавания на основе предсказанного состояния
            for (int i = 0; i < 16; ++i)
                if (currentDeskState[i] != expectedDeskState[i])
                {
                    //  Для плиток со значением больше 4 выбираем предсказанное значение
                    if (expectedDeskState[i] > 2)
                        currentDeskState[i] = expectedDeskState[i];
                    else
                        lethalErrors++;
                }

            if(iterationErrorCount>1)
                errorCount += iterationErrorCount-1;
            if (lethalErrors > 1) stopByErrors = true;
        }

        /// <summary>
        /// Выводим распознанные значения поверх исходного изображения
        /// </summary>
        /// <param name="g"></param>
        /// <param name="blockSide"></param>
        private void DrawNumbersOnOriginalBitmap(Graphics g, int blockSide)
        {
            Font f = new Font(FontFamily.GenericSansSerif, 10);
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    if (currentDeskState[r * 4 + c] >= 1 && currentDeskState[r * 4 + c] <= 16)
                    {
                        int num = 1 << currentDeskState[r * 4 + c];
                        g.DrawString(num.ToString(), f, Brushes.White, settings.margin + c * blockSide, settings.margin + r * blockSide);
                    }
        }

        public void ProcessImage(Bitmap bitmap, bool justShow)
        {
            stopByErrors = false;

            //  «Распиливаем» изображение на 16 фрагментов - по отдельной плитке каждый

            //  Минимальная сторона изображения (обычно это высота)
            if (bitmap.Height > bitmap.Width) throw new Exception("К такой забавной камере меня жизнь не готовила!");
            int side = bitmap.Height;

            //  Отпиливаем границы, но не более половины изображения
            if (side < 4 * settings.border) settings.border = side / 4;
            side -= 2 * settings.border;

            Rectangle cropRect = new Rectangle((bitmap.Width - bitmap.Height) / 2 + settings.left, settings.top, side, side);

            original = new Bitmap(cropRect.Width, cropRect.Height);
            
            //  Рисуем рамки на оригинале 
            Graphics g = Graphics.FromImage(original);

            g.DrawImage(bitmap, new Rectangle(0, 0, original.Width, original.Height), cropRect, GraphicsUnit.Pixel); Pen p = new Pen(Color.Red);

            //  Проверяем значение полей из settings
            if (side < 10 * settings.margin) settings.margin = side / 10;

            //  Высчитываем сторону блока для извлечения
            int sz = side / 4 - 2 * settings.margin;
            int blockSide = side / 4;
            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                    g.DrawRectangle(p, new Rectangle(settings.margin + c * blockSide, settings.margin + r * blockSide, sz, sz));

            //  Рисуем цифры, которые распознались на предыдущем шаге?
            if (justShow) DrawNumbersOnOriginalBitmap(g,blockSide);
            
            //  Конвертируем изображение в градации серого
            AForge.Imaging.Filters.Grayscale grayFilter = new AForge.Imaging.Filters.Grayscale(0.2125, 0.7154, 0.0721);
            processed = grayFilter.Apply(AForge.Imaging.UnmanagedImage.FromManagedImage(original));

            //  Масштабируем изображение до 500x500 - этого достаточно
            AForge.Imaging.Filters.ResizeBilinear scaleFilter = new AForge.Imaging.Filters.ResizeBilinear(500, 500);
            original = scaleFilter.Apply(original);

            //  Если распознавание не планируем – просто выход
            if (justShow) return;

            stopByErrors = false;

            //  Обнуляем текущее состояние
            for (int i = 0; i < 16; ++i)
                currentDeskState[i] = 0;

            //  Пороговый фильтр применяем. Величина порога берётся из настроек, и меняется на форме
            AForge.Imaging.Filters.BradleyLocalThresholding threshldFilter = new AForge.Imaging.Filters.BradleyLocalThresholding();
            threshldFilter.PixelBrightnessDifferenceLimit = settings.differenceLim;
            threshldFilter.ApplyInPlace(processed);

            for (int r = 0; r < 4; ++r)
                for (int c = 0; c < 4; ++c)
                {
                    //  Берём очередной фрагмент - с дополнительными отступами (мы же там нарисовали прямоугольники)
                    AForge.Imaging.Filters.Crop cropFilter = new AForge.Imaging.Filters.Crop(new Rectangle(2 + settings.margin + c * blockSide, 2 + settings.margin + r * blockSide, sz - 4, sz - 4));
                    arrayPics[r, c] = cropFilter.Apply(processed);

                    //arrayPics[r, c] = processed.Clone(new Rectangle(2+settings.margin + c * blockSide, 2+settings.margin + r * blockSide, sz-4, sz-4), processed.PixelFormat);
                    //  И выполняем сопоставление
                    processSample(r, c);
                }
            
            //  Исправляем ошибки
            correctState();
            //  Рисуем полученный результат
            DrawNumbersOnOriginalBitmap(g,blockSide);
        }

        /// <summary>
        /// Обработка одного сэмпла
        /// </summary>
        /// <param name="index"></param>
        private int processSample(int r, int c)
        {
            ///  Инвертируем изображение
            AForge.Imaging.Filters.Invert InvertFilter = new AForge.Imaging.Filters.Invert();
            InvertFilter.ApplyInPlace(arrayPics[r, c]);

            ///    Создаём BlobCounter, выдёргиваем самый большой кусок, масштабируем, пересечение и сохраняем
            ///    изображение в эксклюзивном использовании
            AForge.Imaging.BlobCounterBase bc = new AForge.Imaging.BlobCounter();

            bc.FilterBlobs = true;
            bc.MinWidth = 3;
            bc.MinHeight = 3;
            // Упорядочиваем по размеру
            bc.ObjectsOrder = AForge.Imaging.ObjectsOrder.Size;
            // Обрабатываем картинку

            bc.ProcessImage(arrayPics[r,c]);

            Rectangle[] rects = bc.GetObjectsRectangles();
            if (rects.Length == 0)
            {
                finalPics[r, c] = AForge.Imaging.UnmanagedImage.FromManagedImage(new Bitmap(100, 100));
                return 0;
            }

            // К сожалению, код с использованием подсчёта blob'ов не работает, поэтому просто высчитываем максимальное покрытие
            // для всех блобов - для нескольких цифр, к примеру, 16, можем получить две области - отдельно для 1, и отдельно для 6.
            // Строим оболочку, включающую все блоки. Решение плохое, требуется доработка
            int lx = arrayPics[r,c].Width;
            int ly = arrayPics[r, c].Height;
            int rx = 0;
            int ry = 0;
            for(int i=0;i<rects.Length;++i)
            {
                if (lx > rects[i].X) lx = rects[i].X;
                if (ly > rects[i].Y) ly = rects[i].Y;
                if (rx < rects[i].X + rects[i].Width) rx = rects[i].X + rects[i].Width;
                if (ry < rects[i].Y + rects[i].Height) ry = rects[i].Y + rects[i].Height;
            }

            // Обрезаем края, оставляя только центральные блобчики
            AForge.Imaging.Filters.Crop cropFilter = new AForge.Imaging.Filters.Crop(new Rectangle(lx, ly, rx - lx, ry - ly));
            finalPics[r, c] = cropFilter.Apply(arrayPics[r, c]);

            //  Масштабируем до 100x100
            AForge.Imaging.Filters.ResizeBilinear scaleFilter = new AForge.Imaging.Filters.ResizeBilinear(100, 100);
            finalPics[r, c] = scaleFilter.Apply(finalPics[r, c]);
            
            //  Ну и распознаём
            currentDeskState[r*4+c] = patternMatch(r,c);

            return 0;
        }

        private byte patternMatch(int r, int c)
        {
            //  А теперь перебрать и попробовать наложение с различными заготовками
            AForge.Imaging.Filters.Difference DiffFilter = new AForge.Imaging.Filters.Difference(finalPics[r,c]);

            int notBlack = int.MaxValue;
            byte pwr = 0;
            for (byte i = 1; i < 14; ++i)
            {
                AForge.Imaging.ImageStatistics stat = new AForge.Imaging.ImageStatistics(DiffFilter.Apply(samples[i]));

                if(stat.PixelsCountWithoutBlack<notBlack)
                {
                    notBlack = stat.PixelsCountWithoutBlack;
                    pwr = i;
                }
            }
            return pwr;
        }
    }
}

