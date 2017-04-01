using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Runtime.InteropServices;

namespace AForge.WindowsForms
{
    public class Helper
    {
        [DllImport("CppDll.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Auto)]
        public static extern int solveState(byte[] buff, int buffSize, int depth);
    }

    class Solver
    {
        private bool moveReady = false; 
        public int suggestedMove = 0;

        public byte[] buffer;
        public int bufferSize;

        private int searchDepth;

        public Solver()
        {
            buffer = new byte[16];
            bufferSize = 16;
        }
        
        /// <summary>
        /// Решение состояния и предложение хода
        /// </summary>
        /// <param name="buff">Буфер, представляющий поле</param>
        /// <param name="buffSize">Размер буфера - не используется, равен 16</param>
        /// <param name="depth">Глубина поиска</param>
        public void solveState(byte[] buff, int buffSize, int depth)
        {
            moveReady = false;

            for (int i = 0; i < 16; ++i)
                buffer[i] = buff[i];

            searchDepth = depth;

            Thread trd = new Thread(doWork);
            trd.Start();
        }

        private void doWork()
        {
            suggestedMove = Helper.solveState(buffer, bufferSize, searchDepth);
            moveReady = true;
        }

        public bool workDone() { return moveReady; }

    }
}
