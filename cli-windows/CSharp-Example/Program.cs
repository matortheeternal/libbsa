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
            String error = bsa.bsa_get_error_message();
            if (error != "") {
                Console.WriteLine(error);
                return;
            }
            Console.WriteLine("Opened " + path);

            uint vmajor = bsa.bsa_get_version_major();
            uint vminor = bsa.bsa_get_version_minor();
            uint vpatch = bsa.bsa_get_version_patch();
            Console.WriteLine("BSA Version: " + vmajor + "." + vminor + "." + vpatch);

            string[] entries = bsa.bsa_get_assets(".*");
            error = bsa.bsa_get_error_message();
            if (error != "") {
                Console.WriteLine(error);
                return;
            }
            Console.WriteLine("Found assets:");

            for (int i = 0; i < entries.Length; i++) {
                Console.WriteLine(entries[i]);
            }
        }
    }
}
