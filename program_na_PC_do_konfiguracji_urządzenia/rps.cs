/* Program PC do wprowadzania i odczytu ustawień 
   rejestratora parametrów środowikowychl - Piotr Knutel */
using System;
using System.IO.Ports;
using System.Runtime.InteropServices;

namespace obsluga_rps 
{
    class MainClass {
        enum OSType {
            Windows,
            Linux,
            other
        }

        static OSType CheckOS() {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux)) {
                return OSType.Linux;
            } else if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows)) {
                return OSType.Windows;
            } else {
                return OSType.other;
            }
        }

        public static int Main(string[] args) {
            //byte[] whatTime = new byte[8] {0x77, 0x68, 0x61, 0x74, 0x74, 0x69, 0x6d, 0x65};
            string receivedData = "";
            string programmeName = "Program do komunikacji z rejestartorem parametrów środowiskowych.\nautor: Piotr Knutel, 2021";
            string info = "Wybierz zadanie, poprzez naciśnięcie odpowiedniego klawisza:\n" +
                "  w\t- odczyt aktualnej daty i godziny z rejestratora,\n" +
                "  d\t- ustawianie daty,\n" +
                "  t\t- ustawianie czasu,\n" +
                "  i\t- ustawienie odstępu pomiędzy pomiarami (interwału),\n" +
                "  j\t- odczyt aktualnego odstępu pomiędzy pomiarami (interwału),\n" +
                "  f\t- ustalenie trybu zapisu 'plik dobowy lub tygodniowy',\n" +
                "  g\t- odczyt trybu zapisu 'plik dobowy lub tygodniowy',\n" +
                "  m\t- odczyt informacji o ostatnim pomiarze,\n" +
                "  e\t- zakończ program.";
            string commandWhatTime = "whattime";
            string commandDate = "setdate ";
            string commandGetFileInfo = "gfi";
            OSType os;

            //START

            os = CheckOS();

            Console.ForegroundColor = ConsoleColor.Red;
            Console.WriteLine(programmeName);
            Console.ResetColor();

            SerialPort Serial = new SerialPort();
            Serial.BaudRate = 57600;
            Serial.Parity   = Parity.None;
            Serial.DataBits = 8;
            Serial.StopBits = StopBits.One;

            bool canThePortBeOpened = false;
            string usedPortName;

            while (!canThePortBeOpened) {
                Console.WriteLine("Dostępne porty szeregowe: ");
                foreach (string s in SerialPort.GetPortNames()) {
                    Console.WriteLine("  {0}", s);
                }
                switch (os) {
                    case OSType.Linux :
                        Console.Write("Podaj końcówkę nazwy portu (np. ACM0 lub USB0), lub wprowadź 'e', aby zakończyć program: ");
                        usedPortName = Console.ReadLine();
                        Serial.PortName = "/dev/tty" + usedPortName;
                        break;

                    case OSType.Windows :
                        Console.Write("Podaj numer portu COM (np. 4), lub wprowadź 'e', aby zakończyć program: ");
                        usedPortName = Console.ReadLine();
                        Serial.PortName = "COM" + usedPortName;
                        break;

                    default :
                        Console.Write("Podaj pełną nazwę portu szeregowego (np. COM4 lub /dev/ttyACM0), lub wprowadź 'e', aby zakończyć program: ");
                        usedPortName = Console.ReadLine();
                        Serial.PortName = usedPortName;
                        break;
                }
                if (usedPortName == "e") {
                    return 0;   //zamyka program
                }
                try {
                    Serial.Open();
                    Console.WriteLine("Udało się otworzyć port.");
                    canThePortBeOpened = true;
                } catch (Exception e) {
                    Console.WriteLine("Nie udało się otworzyć portu.");
                    Console.WriteLine(e);
                }
            }
            Console.ForegroundColor = ConsoleColor.DarkYellow;
            Console.WriteLine(info);
            Console.ResetColor();
            
            Serial.Close();

            while (true)
            {
                Console.Write("Wybieram: ");
                ConsoleKeyInfo key = Console.ReadKey();
                Console.WriteLine("");
                
                switch (key.Key) {
                    case ConsoleKey.W:
                        try {
                            Serial.Open();
                            Serial.Write(commandWhatTime);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd W");
                        }
                        break;

                    case ConsoleKey.D:
                        byte year, mounth, day, dayOfWeek;
                        int yearInt;
                        string yearString, mounthString, dayString, dayOfWeekString;

                        Console.Write("Podaj rok: ");
                        yearString = Console.ReadLine();
                        yearInt = Int32.Parse(yearString) - 2000;
                        year = Convert.ToByte(yearInt);

                        Console.Write("Podaj miesiąc (wpisz liczbę od 1 do 12): ");
                        mounthString = Console.ReadLine();
                        mounth = Byte.Parse(mounthString);

                        Console.Write("Podaj dzień miesiąca (od 1 do 31): ");
                        dayString = Console.ReadLine();
                        day = Byte.Parse(dayString);

                        Console.Write("Podaj dzień tygodnia (od 0 do 6, 0 to niedziela, 1 - poniedziałek itd.): ");
                        dayOfWeekString = Console.ReadLine();
                        dayOfWeek = Byte.Parse(dayOfWeekString);

                        try {
                            byte[] d = new byte[4] { year, mounth, day, dayOfWeek };
                            string s = System.Text.Encoding.ASCII.GetString(d);
                            Serial.Open();
                            Serial.Write(commandDate + s);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd D");
                        }
                        break;

                    case ConsoleKey.T:
                        Console.Write("Podaj godzinę: ");
                        string hourString = Console.ReadLine();
                        byte hour = Byte.Parse(hourString);

                        Console.Write("Podaj minutę: ");
                        string minuteString = Console.ReadLine();
                        byte minute = Byte.Parse(minuteString);

                        Console.Write("Podaj sekundę: ");
                        string secondString = Console.ReadLine();
                        byte second = Byte.Parse(secondString);

                        byte[] setTime = new byte[11] { 0x73, 0x65, 0x74, 0x74, 0x69, 0x6d, 0x65, 0x20, hour, minute, second };
                        try {
                            Serial.Open();
                            Serial.Write(setTime, 0, 11);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd T");
                        }
                        break;

                    case ConsoleKey.M:
                        try {
                            Serial.Open();
                            Serial.Write(commandGetFileInfo);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd M");
                        }
                        break;

                    case ConsoleKey.I:
                        Console.WriteLine("Dozwolone wartości interwału [w minutach]: 1, 5, 10, 15, 20, 30, 60.");
                        Console.Write("Podaj interwał: ");
                        string intervalString = Console.ReadLine();
                        byte interval = Byte.Parse(intervalString);

                        byte[] i = new byte[9] { 0x73, 0x65, 0x74, 0x69, 0x6e, 0x74, 0x65, 0x20, interval };
                        try {
                            Serial.Open();
                            Serial.Write(i, 0, 9);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd I");
                        }
                        break;

                    case ConsoleKey.J:
                        byte[] j = new byte[8] { 0x67, 0x65, 0x74, 0x69, 0x6e, 0x74, 0x65, 0x20 };
                        try {
                            Serial.Open();
                            Serial.Write(j, 0, 8);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd J");
                        }
                        break;
                        
                    case ConsoleKey.F:
                        Console.WriteLine("\nPliki z danymi mogą odpowiadać dniom lub tygodniom. Nazwa pliku dziennego to np. M201231A.CSV i odpowiada ona 31 grudniowi 2020. Nazwa pliku tygodniowego to np. M2012W5A.CSV i odpowiada ona piatemu tygodniowi grudnia 2020. Przy czym pierwszy tydzień to zawsze pierwszy PEŁNY tydzień miesiąca, tzn. że jeśli ostatni poniedziałek był w poprzednim miesiącu, to dane będą zapisywane w pliku, odnoszączym się do piątego tygodnia poprzedniego miesiąca.\n\nW celu ograniczenia rozmiaru pliku, w jego nazwie, przed rozszerzeniem .CSV, znajduje się dodatkowy znak. W przypadku, gdy plik osiągnie romzmiar ok. 50 kB, przyszłe dane będą zapisywane w nowym pliku, o nazwie zawierającej następną literę alfabetu.\n");
                        byte daysPerFile = 0;
                        bool correctDaysPerFileValueFlag = false;
                        while(!correctDaysPerFileValueFlag) {
                            Console.Write("Wybierz tryb zapisu (day / week), lub 'e' aly wyjść: ");
                            string daysPerFileString = Console.ReadLine();
                            switch (daysPerFileString) {
                                case "day" :
                                    daysPerFile = 1;
                                    correctDaysPerFileValueFlag = true;
                                    break;
                                case "week" :
                                    daysPerFile = 0;
                                    correctDaysPerFileValueFlag = true;
                                    break;
                                case "e" :
                                    return 0;
                                default :
                                    Console.WriteLine("Nie poprawna wartość. Wpisz słowo: day lub week");
                                    break;
                            }
                        }
                        byte[] f = new byte[9] { 0x73, 0x65, 0x74, 0x64, 0x70, 0x66, 0x73, 0x20, daysPerFile};
                        try {
                            Serial.Open();
                            Serial.Write(f, 0, 9);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd F");
                        }                        
                        break;
                        
                    case ConsoleKey.G:
                        byte[] g = new byte[8] { 0x67, 0x65, 0x74, 0x64, 0x70, 0x66, 0x73, 0x20};
                        try {
                            Serial.Open();
                            Serial.Write(g, 0, 8);
                            receivedData = Serial.ReadLine();
                            Serial.Close();
                            Console.ForegroundColor = ConsoleColor.Cyan;
                            Console.WriteLine(receivedData);
                            Console.ResetColor();
                        } catch {
                            Console.WriteLine("Błąd G");
                        }
                        break;

                    case ConsoleKey.E:
                        return 0;       //zamyka program
                    
                    default:
                        Console.WriteLine("Niepoprawne polecenie.");
                        break;
                }
            }    
        }
    }
}
