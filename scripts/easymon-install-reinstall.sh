#!/bin/bash
## EasyMon Installer Script


# Function prints message, then terminates the script
# with the exit status which was passed to the function.
function exit_script() {
    echo -e "\nExiting...\n"
    exit $1
}

# Function prompts the user to continue, requiring a response of either
# "y" or "n" and re-prompting if needed until valid input is received.
# If the user chooses not to continue, the script teminates.
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

# Name of application and main program.
app='easymon'

# Print informative statement.
echo -e "\nThis is the $app installer."

# Prompt user to continue or exit.
prompt_to_continue

# Ensure that the installer was executed from within its native directory.
# If not, print message then exit immediately.
self=$(basename "${0}")
if [ ! -f $self ]; then
    echo "ERROR! Please execute the installer from within its native directory."
    exit_script 1
fi

# Ensure main program has been compiled.
# If not, print message then exit immediately.
if [ ! -f $app ]; then
    echo "ERROR! Compiled program not found!"
    echo "Before running the installer, please compile $app using the provided Makefile."
    echo "For instructions and additional information, see README.md"
    exit_script 1
fi

# Print informative statement.
echo "Checking compatibility..."

# Set default value of "true" for file system compatibility.
compatible=1

# Check existence required installation directories. If any are missing,
# inform user of each one and update compatibility to "false."
required_dirs=(
    '/usr/bin'
    '/usr/share'
    '/usr/share/icons/hicolor/scalable/apps'
    '/usr/share/applications'
    '/usr/share/doc'
)
for dir in "${required_dirs[@]}"; do
    if [ ! -d "$dir" ]; then
        echo -e "\tERROR! Required directory not found: $dir"
        compatible=0
    fi
done

# Check existence and content of prerequisite kernel files for app compatibility.
# If issue(s) are found, inform user of each one and update compatibility to "false."
prereqs=('/proc/meminfo' '/proc/cpuinfo' '/proc/stat')
for file in "${prereqs[@]}"; do
    if [[ ! -f "$file" ]]; then
        echo -e "\tERROR! Missing required kernel file: $file"
        compatible=0
    elif [[ $file == "${prereqs[0]}" ]]; then
        if [[ ! $(grep -i memavail "$file") ]]; then
            echo -e "\tERROR! File $file does not contain required field: MemAvailable."
            compatible=0
        fi
    fi
done

# If any incompatibilities were found, print message then exit immediately.
if ! ((compatible)); then
    echo -e "\nYour file system does not meet compatibility requirements!"
    echo "For details, please review the error messages above."
    exit_script 1
fi

# Print informative statement.
echo "Checking package contents..."

# Set default value of "false" for missing files.
missing_files=0

# Executeable files to be installed.
executeables=("$app" 'easymon-uninstall.sh')

# All files to be installed and corresponding installed files with path.
# Note that nonexecuteable files are added upon declaration, where
# executeables are then added with /usr/bin/ as their destination.
declare -A to_install=(
    ['easymon.ui']='/usr/share/easymon/ui/easymon.ui'
    ['easymon.css']='/usr/share/easymon/css/easymon.css'
    ['easymon-light-scheme.css']='/usr/share/easymon/css/easymon-light-scheme.css'
    ['easymon-dark-scheme.css']='/usr/share/easymon/css/easymon-dark-scheme.css'
    ['easymon-icon.svg']='/usr/share/icons/hicolor/scalable/apps/easymon-icon.svg'
    ['com.gmail.ctbishop34.easymon.desktop']='/usr/share/applications/com.gmail.ctbishop34.easymon.desktop'
    ['README.txt']='/usr/share/doc/easymon/README.txt'
)
for key in "${executeables[@]}"; do
    to_install["$key"]="/usr/bin/$key"
done

# Ensure all files to be installed are present in the working directory.
# If any are missing, inform user of each one and update missing files to "true."
for file in "${!to_install[@]}"; do
    if [ ! -f "$file" ]; then
        echo -e "\tWARNING! Package file not found: $file"
        missing_files=1
    fi
done

# If any files were missing, prompt user to continue or exit.
# If user does not enter y or n, re-prompt until valid input is received.
if ((missing_files)); then
    echo "Issue(s) were detected! Please review the warning message(s) above."
    prompt_to_continue
fi

# Set default value of "false" for pre-existing installation.
preexisting_installation=0

# Check for pre-existing installation. If any file is found which indicates
# installation, update the value of pre-existing installation to "true."
for file in "${to_install[@]}"; do
    if [ -f "$file" ]; then
        preexisting_installation=1
        break
    fi
done

# Print informative statement.
echo "Continuing to installation."

# If any element of a pre-existing installation was found,
# inform user and prompt to reinstall or exit.
if ((preexisting_installation)); then
    echo "Element(s) of a pre-existing $app installation were found."
    echo "These files will be removed, and $app will be re-installed."
    prompt_to_continue
fi

# If script was not executed as root user, prompt for credentials. If after
# three attempts a valid password is not entered, exit the installer.
if [ ! $(sudo echo 0) ]; then
    echo -e "\nRoot permissions could not be obtained!"
    exit_script 1
fi

# If a pre-existing installation was indicated, remove all of its files.
if ((preexisting_installation)); then
    echo "Removing installed files..."
    for file in "${to_install[@]}"; do
        if [ -f "$file" ]; then
            if sudo rm "$file" && [ ! -f "$file" ]; then
                echo -e "\t$file"
            else
                echo -e "\tWARNING! Could not remove file: $file"
            fi
        fi
    done
fi

# Print informative statement.
echo "Creating new directories..."

# Set default value of "true" for new directories sucessfully created.
new_dirs_success=1

# Create directories for resource files, printing each new path.
# If return value indicates failure for the creation of any directory,
# inform user and update the value of new dirs success to "false."
new_dirs=(
    '/usr/share/easymon/ui'
    '/usr/share/easymon/css'
    '/usr/share/doc/easymon/'
)
for dir in "${new_dirs[@]}"; do
    if sudo mkdir -p "$dir" &> /dev/null; then
        echo -e "\t$dir"
    else
        echo "WARNING! Could not create directory: $dir"
        new_dirs_success=0
    fi
done

# If any directories could not be created,
# print message and prompt user to continue or exit.
if ! ((new_dirs_success)); then
    echo "Failed to create directories! Please review the warning messages above."
    prompt_to_continue
fi

# Print informative statement.
echo "Installing files..."

# Set default value of "false" for the detection of any issues through
# the remainder of the installation process (remainder of this script).
installation_warning=0

# Install files, printing the file names and path for each.
# If return value indicates failure for installation of any file,
# inform user and update the value of installation warning to "true."
for file in "${!to_install[@]}"; do
    result="${to_install[$file]}"
    if sudo cp "$file" "$result" &> /dev/null && [ -f "$result" ]; then
        echo -e "\t$result"
    else
        echo -e "\tWARNING! Could not install: $result"
        installation_warning=1
    fi
done

# Print informative statement.
echo "Giving executeables permission to execute..."

# Grant installed executeables (main program and uninstaller) permission to
# execute. If return value indicates failure and file does not already have
# permission to execute, update the value of installation warning to "true."
for file in "${executeables[@]}"; do
    result="${to_install[$file]}"
    if sudo chmod +x "$result" &> /dev/null && [ -x "$result" ]; then
        echo -e "\t$result"
    else
        echo -e "\tWARNING! Could not grant permission to execute: $result"
        installation_warning=1
    fi
done

# Print informative statement.
echo -e -n "Updating GTK-4 icon cache...\n\t"

# Update GTK-4 icon cache. If the utility needed to do this is unavailable,
# update the value of installation warning to "true."
# Otherwise, allow the output (if any) to print to the terminal.
utility='gtk4-update-icon-cache'
if command -v $utility &> /dev/null;then
    sudo $utility /usr/share/icons/hicolor
else
    echo "WARNING! Could not update icon cache - utility unavailable: $utility"
    installation_warning=1
fi

# If any warning messages were printed during installation,
# prompt user to review them. Otherwise, print success message.
echo ""
if ((installation_warning)); then
    echo "IMPORTANT - Issue(s) occured during installation!"
    echo "Please review the warning messages above."
else
    echo "Installation is complete!"
    echo "The downloaded files are no longer needed, and may be removed."
fi
echo ""

