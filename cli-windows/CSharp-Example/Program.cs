using System;
using libbsa;
using System.IO;

namespace CSharp_Example {
    class Program {
        static BSANET bsa = new BSANET();

        static void Main(string[] args) {
            if (args.Length == 0) {
                Console.WriteLine("Please enter a path to a BSA file to get contents from.");
                return;
            }

            String path = args[0];
            if (!File.Exists(path)) {
                Console.WriteLine("No file found at path: " + path);
                return;
            }

            bsa.bsa_open(path);
            String error = "";
            if (bsa.bsa_get_error_message(error) != 0) {
                Console.WriteLine(error);
                return;
            }
            Console.WriteLine("Opened " + path);

            string[] entries = new string[0];
            bsa.bsa_get_assets("", entries);
            if (bsa.bsa_get_error_message(error) != 0) {
                Console.WriteLine(error);
                return;
            }
            Console.WriteLine("Found assets:");

            for (int i = 0; i < entries.Length; i++) {
                Console.WriteLine(entries[i]);
            }
            return;
        }
    }
}
