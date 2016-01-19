﻿using System.Collections.Generic;
using System.Drawing;
using System.IO.Ports;

namespace AmbiBeam
{
    public class Communication
    {
        private readonly SerialPort _port;
        public Communication(string portname)
        {
            _port = new SerialPort(portname, 115200);
            _port.Open();
        }

        public void Write(List<Color> colors, byte brightness = 0x4F)
        {


            byte[] buffer = new byte[10];
            buffer[0] = 1; // SOH
            buffer[1] = (byte)(0xff & colors.Count);
            buffer[2] = (byte)((colors.Count >> 8) & 0xff);
            buffer[3] = brightness;
            buffer[4] = 2; // STX
            _port.Write(buffer, 0, 4);

            foreach (Color color in colors)
            {
                buffer[0] = color.R;
                buffer[1] = color.G;
                buffer[2] = color.B;

                _port.Write(buffer, 0, 3);
            }
            buffer[0] = 3; // ETX
            _port.Write(buffer, 0, 1);
        }

        ~Communication()
        {
            _port.Close();
        }
    }


}
