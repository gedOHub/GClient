using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using NUnit.Framework;
using SctpDrv;
using System.Net;
using System.Net.Sockets;
using System.IO;

namespace SctpDrv
{
    [TestFixture]
    public class SctpTest
    {
        [TestCase]
        public void BasicTxRx1()
        {
            SctpSocket listenSocket = new SctpSocket(AddressFamily.InterNetwork, SocketType.Stream);
            SctpSocket clientSocket = new SctpSocket(AddressFamily.InterNetwork, SocketType.Stream);

            listenSocket.Bind(new IPEndPoint(IPAddress.Parse("127.0.0.1"), 10000));
            listenSocket.Listen(10);

            clientSocket.Connect("127.0.0.1", 10000);
            SctpSocket serverSocket = listenSocket.Accept();

            byte[] buffer = new byte[256];
            const string testString = "BasicTxRx1";

            serverSocket.Send(new ASCIIEncoding().GetBytes(testString));
            clientSocket.Receive(buffer);

            string result = new ASCIIEncoding().GetString(buffer).TrimEnd(new char[] { '\0' });
            NUnit.Framework.StringAssert.AreEqualIgnoringCase(testString, result);
        }
    }
}
