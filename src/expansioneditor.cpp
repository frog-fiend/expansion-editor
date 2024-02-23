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

// Expansion pack info
typedef struct {
   std::string otherPathName;
   std::string eaPathName;
} ExpansionPack;

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
void activatePack(std::vector<ExpansionPack> &activePacks, std::vector<ExpansionPack> &inactivePacks)
{
   // Edge case
   if (inactivePacks.empty())
   {
      std::cout << "No inactive packs.\n\n";
      return;
   }

   // For handling exit
   std::vector<HKEY> openedKeys;

   // Get user choice
   std::cout << "\n";
   for (int i = 1; i <= inactivePacks.size(); i++)
   {
      std::cout << i << ": " << inactivePacks[i - 1].eaPathName << "\n";
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
      goto exit;
   }
   openedKeys.push_back(otherBackupKey);

   result = RegOpenKeyExA(HKEYROOT, (backupPaths[1]).c_str(), 0, KEY_ALL_ACCESS, &eaBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key eaBackupKey: " << result << std::endl;
      goto exit;
   }
   openedKeys.push_back(eaBackupKey);

   HKEY otherRealKey;
   HKEY eaRealKey;
   DWORD creationStatus;
   result = RegCreateKeyExA(HKEYROOT, (realPaths[0] + "\\" + inactivePacks[packIndex].otherPathName).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &otherRealKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      goto exit;
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      goto exit;
   }
   openedKeys.push_back(otherRealKey);

   result = RegCreateKeyExA(HKEYROOT, (realPaths[1] + "\\" + inactivePacks[packIndex].eaPathName).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &eaRealKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      goto exit;
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      goto exit;
   }
   openedKeys.push_back(eaRealKey);

   // Copy to real keys
   result = RegCopyTreeA(otherBackupKey, inactivePacks[packIndex].otherPathName.c_str(), otherRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      goto exit;
   }
   result = RegCopyTreeA(eaBackupKey, inactivePacks[packIndex].eaPathName.c_str(), eaRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      goto exit;
   }

   // Delete inactive keys
   result = RegDeleteKeyExA(otherBackupKey, inactivePacks[packIndex].otherPathName.c_str(), KEY_WOW64_32KEY, 0);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key otherBackupKey: " << result << std::endl;
      goto exit;
   }
   result = RegDeleteTreeA(eaBackupKey, inactivePacks[packIndex].eaPathName.c_str());
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key eaBackupKey: " << result << std::endl;
      goto exit;
   }

   // Move pack to active list
   activePacks.push_back(inactivePacks[packIndex]);
   inactivePacks.erase(inactivePacks.begin() + packIndex);

exit:
   // Close HKEYs
   for(HKEY hKey : openedKeys)
   {
      RegCloseKey(hKey);
   }

   // If we have gotten here with an error; fatal error has occured
   if(result != ERROR_SUCCESS)
   {
      std::cout << "Fatal error occured." << std::endl;
      exit(1);
   }

}

/// @brief  Prompts user and deactivates selected pack
/// @param  activePacks   Currently active packs
/// @param  inactivePacks Currently inactive packs
void deactivatePack(std::vector<ExpansionPack> &activePacks, std::vector<ExpansionPack> &inactivePacks)
{
   // Edge case
   if (activePacks.empty())
   {
      std::cout << "No active packs.\n\n";
      return;
   }

   // For handling exit
   std::vector<HKEY> openedKeys;

   // Get user choice
   std::cout << "\n";
   for (int i = 1; i <= activePacks.size(); i++)
   {
      std::cout << i << ": " << activePacks[i - 1].eaPathName << "\n";
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
      goto exit;
   }
   openedKeys.push_back(otherRealKey);

   result = RegOpenKeyExA(HKEYROOT, (realPaths[1]).c_str(), 0, KEY_ALL_ACCESS, &eaRealKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key eaRealKey: " << result << std::endl;
      goto exit;
   }
   openedKeys.push_back(eaRealKey);

   HKEY otherBackupKey;
   HKEY eaBackupKey;
   DWORD creationStatus;
   result = RegCreateKeyExA(HKEYROOT, (backupPaths[0] + "\\" + activePacks[packIndex].otherPathName).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &otherBackupKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      goto exit;
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      goto exit;
   }
   openedKeys.push_back(otherBackupKey);

   result = RegCreateKeyExA(HKEYROOT, (backupPaths[1] + "\\" + activePacks[packIndex].eaPathName).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &eaBackupKey, &creationStatus);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error creating key: " << result << std::endl;
      goto exit;
   }
   if (creationStatus != REG_CREATED_NEW_KEY)
   {
      std::cout << "Error creating key: " << creationStatus << std::endl;
      goto exit;
   }
   openedKeys.push_back(eaBackupKey);


   // Copy to backup keys
   result = RegCopyTreeA(otherRealKey, activePacks[packIndex].otherPathName.c_str(), otherBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      goto exit;
   }
   result = RegCopyTreeA(eaRealKey, activePacks[packIndex].eaPathName.c_str(), eaBackupKey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error copying key: " << result << std::endl;
      goto exit;
   }

   // Delete active keys
   result = RegDeleteKeyExA(otherRealKey, activePacks[packIndex].otherPathName.c_str(), KEY_WOW64_32KEY, 0);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key otherRealKey: " << result << std::endl;
      goto exit;
   }
   result = RegDeleteTreeA(eaRealKey, activePacks[packIndex].eaPathName.c_str());
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error deleting key eaRealKey: " << result << std::endl;
      goto exit;
   }

   // Move pack to inactive list
   inactivePacks.push_back(activePacks[packIndex]);
   activePacks.erase(activePacks.begin() + packIndex);

exit:
   // Close HKEYs
   for(HKEY hKey : openedKeys)
   {
      RegCloseKey(hKey);
   }

   // If we have gotten here with an error; fatal error has occured
   if(result != ERROR_SUCCESS)
   {
      std::cout << "Fatal error occured." << std::endl;
      exit(1);
   }
}

/// @brief  Prompts user and activates selected pack
/// @param  activePacks   Currently active packs
/// @param  inactivePacks Currently inactive packs
void showStatus(std::vector<ExpansionPack> &activePacks, std::vector<ExpansionPack> &inactivePacks)
{
   std::cout << "==== ACTIVE PACKS ====\n";
   for (int i = 1; i <= activePacks.size(); i++)
   {
      std::cout << i << ": " << activePacks[i - 1].eaPathName << "\n";
   }
   std::cout << "\n";
   std::cout << "==== DEACTIVATED PACKS ====\n";
   for (int i = 1; i <= inactivePacks.size(); i++)
   {
      std::cout << i << ": " << inactivePacks[i - 1].eaPathName << "\n";
   }
}

/// @brief  Loads packs from key register
/// @param  activePacks   Vector to store active packs
/// @param  inactivePacks Vector to store inactive packs
void loadPacks(std::vector<ExpansionPack> &activePacks, std::vector<ExpansionPack> &inactivePacks)
{
   // Init variables
   HKEY activeOtherHkey;
   HKEY activeEaHkey;
   HKEY deactiveOtherHkey;
   HKEY deactiveEaHkey;
   LONG result;
   std::vector<HKEY> openedKeys; // Simplifies closing and error handling

   int subkeyIndex = 0;
   DWORD bufferSize = 255;
   char bufferOther[bufferSize];
   char bufferEa[bufferSize];

   // Fetch parent key access
   result = RegOpenKeyExA(HKEYROOT, realPaths[0].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &activeOtherHkey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key: " << result << std::endl;
      goto exit;
   }
   openedKeys.push_back(activeOtherHkey);

   result = RegOpenKeyExA(HKEYROOT, realPaths[1].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &activeEaHkey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key: " << result << std::endl;
      goto exit;
   }
   openedKeys.push_back(activeEaHkey);

   result = RegOpenKeyExA(HKEYROOT, backupPaths[0].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &deactiveOtherHkey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key: " << result << std::endl;
      goto exit;
   }
   openedKeys.push_back(deactiveOtherHkey);

   result = RegOpenKeyExA(HKEYROOT, backupPaths[1].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &deactiveEaHkey);
   if (result != ERROR_SUCCESS)
   {
      std::cout << "Error opening key: " << result << std::endl;
      goto exit;
   }
   openedKeys.push_back(deactiveEaHkey);

   // Find all subkeys/packs
   while (true)
   {
      result = RegEnumKeyExA(activeOtherHkey, subkeyIndex, bufferOther, &bufferSize, NULL, NULL, NULL, NULL);
      bufferSize = 255; // Need to reset as RegEnumKeyExA changes bufferSize
      result = result == ERROR_SUCCESS ? RegEnumKeyExA(activeEaHkey, subkeyIndex, bufferEa, &bufferSize, NULL, NULL, NULL, NULL) : result;
      if (result != ERROR_SUCCESS)
      {
         if (result == ERROR_NO_MORE_ITEMS)
         {
            break;
         }
         else
         {
            goto exit;
         }
      }

      // Add both reg key names of pack
      ExpansionPack pack = { bufferOther, bufferEa };
      activePacks.push_back(pack);
      bufferSize = 255;
      subkeyIndex++;
   }

   subkeyIndex = 0;
   while (true)
   {
      result = RegEnumKeyExA(deactiveOtherHkey, subkeyIndex, bufferOther, &bufferSize, NULL, NULL, NULL, NULL);
      bufferSize = 255;
      result = result == ERROR_SUCCESS ? RegEnumKeyExA(deactiveEaHkey, subkeyIndex, bufferEa, &bufferSize, NULL, NULL, NULL, NULL) : result;
      if (result != ERROR_SUCCESS)
      {
         if (result == ERROR_NO_MORE_ITEMS)
         {
            break;
         }
         else
         {
            std::cout << "Error reading subkeys: " << result << std::endl;
            goto exit;
         }
      }

      // Add both reg key names of pack
      ExpansionPack pack = { bufferOther, bufferEa };
      inactivePacks.push_back(pack);
      bufferSize = 255;
      subkeyIndex++;
   }

exit:
   // Close HKeys
   for(HKEY hKey : openedKeys)
   {
      RegCloseKey(hKey);
   }

   // If we have gotten here with an error; fatal error has occured
   if(result != ERROR_NO_MORE_ITEMS)
   {
      std::cout << "Fatal error occured." << std::endl;
      exit(1);
   }
}

/// @brief  Ensures that a backup registry key exists
void assertBackupKey()
{
   // Prep storage
   HKEY otherBackupKey;
   HKEY eaBackupKey;
   LONG result;
   DWORD creationStatus;

   // Fetch parent key access
   result = RegOpenKeyExA(HKEYROOT, backupPaths[0].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &otherBackupKey);
   result = result == ERROR_SUCCESS ? RegOpenKeyExA(HKEYROOT, backupPaths[1].c_str(), 0, KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS, &eaBackupKey) : result;
   if (result != ERROR_SUCCESS)
   {
      if (result == ERROR_FILE_NOT_FOUND)
      {
         // Ask for permission to create backup registry key
         std::cout << "No backup data found. Create new backup key?: [y/n] " << result << std::endl;
         char input = 'x';
         while (input != 'y' && input != 'n')
         {
            input = getInput<char>();
         }

         if (input == 'y')
         {
            
            result = RegCreateKeyExA(HKEYROOT, (backupPaths[0]).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &otherBackupKey, &creationStatus);
            result = result == ERROR_SUCCESS ? RegCreateKeyExA(HKEYROOT, (backupPaths[1]).c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &eaBackupKey, &creationStatus) : result;
         }
         else
         {
            std::cout << "Error creating key: " << result << std::endl;
            RegCloseKey(otherBackupKey);
            RegCloseKey(eaBackupKey);
            exit(1);
         }
      }
      else
      {
         std::cout << "Error opening key: " << result << std::endl;
         RegCloseKey(otherBackupKey);
         RegCloseKey(eaBackupKey);
         exit(1);
      }
   }

   RegCloseKey(otherBackupKey);
   RegCloseKey(eaBackupKey);
}

int main(int argc, char *argv[])
{
   // Init
   bool quit = false;
   char input = NOTDEF;
   std::vector<ExpansionPack> activePacks;
   std::vector<ExpansionPack> inactivePacks;
   assertBackupKey();
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
