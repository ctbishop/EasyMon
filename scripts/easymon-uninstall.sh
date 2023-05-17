#!/bin/bash
## EasyMon Installer Script


# Function prints message, then terminates the script
# with the exit status which was passed to the function.
function exit_script() {
    echo -e "\nExiting...\n"
    exit $1
}

# Function prompts the user to continue, requiring a response of
# either y or n and re-prompting if needed until valid input is
# received. If the user chooses not to continue, the script teminates.
function prompt_to_continue() {
echo -n "Continue? (y/n) "
read
until [[ $REPLY = 'y' || $REPLY = 'n' ]]; do
    echo -n "Enter y or n: "
    read
done
if [ $REPLY = 'n' ]; then
    exit_script 0
fi
}

# Application and program name.
app='easymon'

# Print informative statement.
echo -e "\nThis is the $app uninstaller."

# Prompt user to continue or exit.
prompt_to_continue

# Check if app is currently running by attempting to get its process ID. 
# If the result is nonempty, inform user then exit immediately.
process_id=$(ps -e | grep "$program" | awk '{print $0" "}' | grep " easymon " | awk '{print $1}')
if [ ! -z $process_id ];then
    echo -e "\nAn instance of $app is currently running with process ID $process_id."
    echo "Please close $app before running the uninstaller."
    exit_script 1
fi

# If script was not executed as root user, prompt for credentials.
if [ ! $(sudo echo 0) ];then
    exit_script 1
fi

# Print informative message.
echo "Uninstalling files..."

# expected installed files to be removed, not including self.
to_remove=(
    "/usr/bin/$app"
    '/usr/share/easymon/ui/easymon.ui'
    '/usr/share/easymon/css/easymon.css'
    '/usr/share/easymon/css/easymon-light-scheme.css'
    '/usr/share/easymon/css/easymon-dark-scheme.css'
    '/usr/share/icons/hicolor/scalable/apps/easymon-icon.svg'
    '/usr/share/applications/com.gmail.ctbishop34.easymon.desktop'
    '/usr/share/doc/easymon/README.txt'
)

# Set default value of "true" for successful removal of all installed files.
success=1

# Remove any installed files, printing the file name and path for
# each. If return value indicates failure for the removal of any
# file, inform user and update the value of success to "false."
for file in "${to_remove[@]}"; do
    if [ -f "$file" ]; then
        if sudo rm "$file" && [ ! -f "$file"} ]; then
            echo -e "\t$file"
        else
            echo -e "\tWARNING! Could not remove file: $files"
            success=0
        fi
    fi
done

# If any files could not be removed, print message then exit immediately.
if ! ((success)); then
    echo -e "\nIMPORTANT - Issue(s) occurred during uninstallation!"
    echo "Please review the warning messages above."
    exit_script 1
fi

# Print informative message.
echo "$app was uninstalled."

# Prompt user to finalize uninstallation or exit.
echo "The final step will remove this uninstaller."
prompt_to_continue

# If user chose to finalize uninstallation,
# remove self (uninstall this script) on exit.
trap "sudo rm ${0}" EXIT

