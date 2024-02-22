#include <Windows.h>
#include <cstring>
#include <iostream>
#include <vector>
#include <limits>

#define HKEYROOT HKEY_LOCAL_MACHINE

// Menu options
enum Options
{
   NOTDEF,
   ACTIVATE = 'a',
   DEACTIVATE = 'd',
   SHOWSTATUS = 's',
   EXIT = 'e'
};

// Absolute paths to registry keys
const std::vector<std::string> realPaths = {"SOFTWARE\\WOW6432Node\\Sims(Steam)", "SOFTWARE\\WOW6432Node\\Electronic Arts\\Sims(Steam)"};
const std::vector<std::string> backupPaths = {"SOFTWARE\\WOW6432Node\\Sims.BK", "SOFTWARE\\WOW6432Node\\Electronic Arts\\Sims.BK"};

/// @brief Displays menu
void displayMenu()
{
   std::cout << "\n\nOPTIONS:\n";
   std::cout << "(A)ctivate\n";
   std::cout << "(D)eactivate\n";
   std::cout << "(S)how status\n";
   std::cout << "(E)xit\n";
   std::cout << "\n\nPICK AN OPTION: ";
}

/// @brief   Gets user input. Reprompts user until input of type T is given.
/// @returns User input of type T
template <class T>
T getInput()
{
   T val = {};
   bool error = false;
   do
   {
      std::cin >> val;
      if (error = std::cin.fail())
      {
         std::cin.clear();
         std::cout << "Invalid input.\n";
      }
      std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
   } while (error);

   return val;
}

/// @brief  Prompts user and activates selected pack 
/// @param  activePacks   Currently active packs
/// @param  inactivePacks Currently inactive packs
void activatePack(std::vector<std::string> &activePacks, std::vector<std::string> &inactivePacks)
{
      // Edge case
   if (inactivePacks.empty())
   {
      std::cout << "No inactive packs.\n\n";
      return;
   }

   // Get user choice
   std::cout << "\n";
   for (int i = 1; i <= inactivePacks.size(); i++)
   {
      std::cout << i << ": " << inactivePacks[i - 1] << "\n";
   }

   std::cout << "\n==== Pick pack by list index ====\n";
   int packIndex = -1;
   packIndex = getInput<int>();
   if (packIndex < 1 || packIndex > inactivePacks.size())
   {
      std::cout << "Unrecognised index.\n\n";
      return;
   }
   packIndex--;

   // Open HKEYS
   HKEY otherBackupKey;
   HKEY eaBackupKey;
   auto result = RegOpenKeyExA(HKEYROOT, (backupPaths[0]).c_str(), 0, KEY_ALL_ACCESS, &otherBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key otherBackupKey: " << result << std::endl;
      exit(1);
   }
   result = RegOpenKeyExA(HKEYROOT, (backupPaths[1]).c_str(), 0, KEY_ALL_ACCESS, &eaBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key eaBackupKey: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      exit(1);
   }

   HKEY otherRealKey;
   HKEY eaRealKey;
   DWORD creationStatus;
   result = RegCreateKeyExA(HKEYROOT, (realPaths[0] + "\\" + inactivePacks[packIndex]).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &otherRealKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      exit(1);
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      exit(1);
   }
   result = RegCreateKeyExA(HKEYROOT, (realPaths[1] + "\\" + inactivePacks[packIndex]).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &eaRealKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      exit(1);
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      exit(1);
   }

   // Copy to real keys
   result = RegCopyTreeA(otherBackupKey, inactivePacks[packIndex].c_str(), otherRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      exit(1);
   }
   result = RegCopyTreeA(eaBackupKey, inactivePacks[packIndex].c_str(), eaRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      exit(1);
   }

   // Delete inactive keys
   result = RegDeleteKeyExA(otherBackupKey, inactivePacks[packIndex].c_str(), KEY_WOW64_32KEY, 0);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key otherBackupKey: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      exit(1);
   }
   result = RegDeleteTreeA(eaBackupKey, inactivePacks[packIndex].c_str());
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key eaBackupKey: " << result << std::endl;
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      exit(1);
   }

   // Close HKEYs
   RegCloseKey(otherBackupKey);
   RegCloseKey(eaBackupKey);
   RegCloseKey(otherRealKey);
   RegCloseKey(eaRealKey);

   // Move pack to active list
   activePacks.push_back(inactivePacks[packIndex]);
   inactivePacks.erase(inactivePacks.begin() + packIndex);
}

/// @brief  Prompts user and deactivates selected pack 
/// @param  activePacks   Currently active packs
/// @param  inactivePacks Currently inactive packs
void deactivatePack(std::vector<std::string> &activePacks, std::vector<std::string> &inactivePacks)
{
   // Edge case
   if (activePacks.empty())
   {
      std::cout << "No active packs.\n\n";
      return;
   }

   // Get user choice
   std::cout << "\n";
   for (int i = 1; i <= activePacks.size(); i++)
   {
      std::cout << i << ": " << activePacks[i - 1] << "\n";
   }

   std::cout << "\n==== Pick pack by list index ====\n";
   int packIndex = -1;
   packIndex = getInput<int>();
   if (packIndex < 1 || packIndex > activePacks.size())
   {
      std::cout << "Unrecognised index.\n\n";
      return;
   }
   packIndex--;

   // Open HKEYS
   HKEY otherRealKey;
   HKEY eaRealKey;
   auto result = RegOpenKeyExA(HKEYROOT, (realPaths[0]).c_str(), 0, KEY_ALL_ACCESS, &otherRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key otherRealKey: " << result << std::endl;
      exit(1);
   }
   result = RegOpenKeyExA(HKEYROOT, (realPaths[1]).c_str(), 0, KEY_ALL_ACCESS, &eaRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key eaRealKey: " << result << std::endl;
      RegCloseKey(otherRealKey);
      exit(1);
   }

   HKEY otherBackupKey;
   HKEY eaBackupKey;
   DWORD creationStatus;
   result = RegCreateKeyExA(HKEYROOT, (backupPaths[0] + "\\" + activePacks[packIndex]).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &otherBackupKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      exit(1);
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      exit(1);
   }
   result = RegCreateKeyExA(HKEYROOT, (backupPaths[1] + "\\" + activePacks[packIndex]).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &eaBackupKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      exit(1);
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      exit(1);
   }

   // Copy to backup keys
   result = RegCopyTreeA(otherRealKey, activePacks[packIndex].c_str(), otherBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      exit(1);
   }
   result = RegCopyTreeA(eaRealKey, activePacks[packIndex].c_str(), eaBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      exit(1);
   }

   // Delete active keys
   result = RegDeleteKeyExA(otherRealKey, activePacks[packIndex].c_str(), KEY_WOW64_32KEY, 0);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key otherRealKey: " << result << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      exit(1);
   }
   result = RegDeleteTreeA(eaRealKey, activePacks[packIndex].c_str());
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key eaRealKey: " << result << std::endl;
      RegCloseKey(otherRealKey);
      RegCloseKey(eaRealKey);
      RegCloseKey(otherBackupKey);
      RegCloseKey(eaBackupKey);
      exit(1);
   }

   // Close HKEYs
   RegCloseKey(otherRealKey);
   RegCloseKey(eaRealKey);
   RegCloseKey(otherBackupKey);
   RegCloseKey(eaBackupKey);

   // Move pack to inactive list
   inactivePacks.push_back(activePacks[packIndex]);
   activePacks.erase(activePacks.begin() + packIndex);
}

/// @brief  Prompts user and activates selected pack 
/// @param  activePacks   Currently active packs
/// @param  inactivePacks Currently inactive packs
void showStatus(std::vector<std::string> &activePacks, std::vector<std::string> &inactivePacks)
{
   std::cout << "==== ACTIVE PACKS ====\n";
   for (int i = 1; i <= activePacks.size(); i++)
   {
      std::cout << i << ": " << activePacks[i - 1] << "\n";
   }
   std::cout << "\n";
   std::cout << "==== DEACTIVATED PACKS ====\n";
   for (int i = 1; i <= inactivePacks.size(); i++)
   {
      std::cout << i << ": " << inactivePacks[i - 1] << "\n";
   }
}

/// @brief  Loads pack from key register 
/// @param  activePacks   Vector to store active packs
/// @param  inactivePacks Vector to store inactive packs
void loadPacks(std::vector<std::string> &activePacks, std::vector<std::string> &inactivePacks)
{
   // Prep storage
   HKEY activeHkey;
   HKEY deactiveHkey;
   LONG result;

   // Fetch parent key access
   result = RegOpenKeyExA(HKEYROOT, realPaths[0].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &activeHkey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key: " << result << std::endl;
      exit(1);
   }
   result = RegOpenKeyExA(HKEYROOT, backupPaths[0].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &deactiveHkey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key: " << result << std::endl;
      RegCloseKey(activeHkey);
      exit(1);
   }

   // Find all subkeys/packs
   int subkeyIndex = 0;
   DWORD bufferSize = 255;
   char buffer[bufferSize];
   while (true)
   {
      result = RegEnumKeyExA(activeHkey, subkeyIndex, buffer, &bufferSize, NULL, NULL, NULL, NULL);
      if (result != ERROR_SUCCESS)
      {
         if (result == ERROR_NO_MORE_ITEMS)
         {
            break;
         }
         else
         {
            std::cout << "Error reading subkeys: " << result << std::endl;
            RegCloseKey(activeHkey);
            RegCloseKey(deactiveHkey);
            exit(1);
         }
      }
      activePacks.push_back(buffer);
      bufferSize = 255;
      subkeyIndex++;
   }
   subkeyIndex = 0;
   while (true)
   {
      result = RegEnumKeyExA(deactiveHkey, subkeyIndex, buffer, &bufferSize, NULL, NULL, NULL, NULL);
      if (result != ERROR_SUCCESS)
      {
         if (result == ERROR_NO_MORE_ITEMS)
         {
            break;
         }
         else
         {
            std::cout << "Error reading subkeys: " << result << std::endl;
            RegCloseKey(activeHkey);
            RegCloseKey(deactiveHkey);
            exit(1);
         }
      }
      bufferSize = 255;
      inactivePacks.push_back(buffer);
      subkeyIndex++;
   }

   // Close HKeys
   RegCloseKey(activeHkey);
   RegCloseKey(deactiveHkey);
}

int main(int argc, char *argv[])
{
   // Init
   bool quit = false;
   char input = NOTDEF;
   std::vector<std::string> activePacks;
   std::vector<std::string> inactivePacks;
   loadPacks(activePacks, inactivePacks);

   // Menu loop
   while (!quit)
   {
      displayMenu();
      input = getInput<char>();
      switch (std::tolower(input))
      {
      case ACTIVATE:
         activatePack(activePacks, inactivePacks);
         break;

      case DEACTIVATE:
         deactivatePack(activePacks, inactivePacks);
         break;

      case SHOWSTATUS:
         showStatus(activePacks, inactivePacks);
         break;

      case EXIT:
         exit(0);
         break;

      default:
         std::cout << "Unknown command.";
         break;
      }
   }
}
