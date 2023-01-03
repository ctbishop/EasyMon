#!/bin/bash

##########################################################################################
#                                         About                                          #
##########################################################################################
#  Package installation script for "EasyMon" system monitor app.
#
#     Author Contact: ctbishop34@gmail.com
#
#     PLEASE NOTE:
#     Although written with the best intentions, this script is not guaranteed to
#     function as intended or to produce the any particular result. The execution of
#     this script is thus at your own risk. Please report bugs and other issues to 
#     ctbishop34@gmail.com.


##########################################################################################
#                                         Setup                                          #
##########################################################################################

app='EasyMon'
script='easymon-install-reinstall.sh'


## CHECK/GET ROOT PRIVILEGES.
echo -e "\n$app installer for Linux OS.\n"  #informative statement.
if [ ! $(sudo echo 0) ];then  #check/prompt
    exit
fi

## CONFIRM SELF IN WORKING DIRECTORY.
if [ ! -f $script ];then
    echo "Error! Please execute $script from its native directory."
    exit
fi


## FILES & DIRECTORIES.
declare -a prereqs  #prerequisite kernel files
    prereqs[0]='/proc/meminfo'
    prereqs[1]='/proc/cpuinfo'
    prereqs[2]='/proc/stat'
declare -a files  #package files
    files[0]='easymon'
    files[1]='easymon-uninstall.sh'
    files[2]='easymon-icon.svg'
    files[3]='com.gmail.ctbishop34.EasyMon.desktop'
    files[4]='README.txt'
declare -a dirs  #destination directories
    dirs[0]='/usr/bin'
    dirs[1]='/usr/bin'
    dirs[2]='/usr/share/icons/hicolor/scalable/apps'
    dirs[3]='/usr/share/applications'
declare -a result  #expected installation results
    for ((a=0;a<4;a++));do
        result[${a}]=${dirs[${a}]}/${files[${a}]}  #file at location
        if [ $a -eq 1 ];then  # is uninstaller
            result[${a}]=${result[${a}]%.*}  # remove extension
        fi
    done


##########################################################################################
#                            Check Compatibility & Readiness                             #
##########################################################################################

installed=0
warning=0
icon=1
unk=0


## CHECK FILES & DIRECTORIES.
echo -e "\nChecking files and directories..."

    ## Affirmative Messages.
for ((a=0;a<4;a++));do
    if [ -f ${files[${a}]} ];then
        echo -e "\t found package file: ${files[${a}]}"
    fi
done
for ((a=0;a<4;a++));do
    if [ -d ${dirs[${a}]} ];then
        echo -e "\t found directory: ${dirs[${a}]}"
    fi
done

    ## Warning Messages.
for ((a=0;a<4;a++));do
    if [ -f ${result[${a}]} ];then  #pre-existing installation
        installed=1  #(line 155 for message)
    fi
    if [[ $a -lt 3 && ! -f ${prereqs[${a}]} ]];then  #missing file for compatibility
        echo -e "\tWARNING! (incompatibility) kernel file not found: ${prereqs[${a}]}"
        warning=1
    elif [[ $a -eq 0 && -f ${prereqs[${a}]} ]];then  #meminfo found
        if [[ ! $(grep -i memavail ${prereqs[${a}]}) ]];then  #missing field
            echo -e "\tWARNING! (incompatibility) kernel file missing required field: ${prereqs[${a}]}, MemAvailable"
            warning=1
    fi
        fi
    if [[ ! -f ${files[${a}]} || ! -d ${dirs[${a}]} ]];then
        warning=1
        if [ ! -f ${files[${a}]} ];then  #missing package file
            echo -e "\tWARNING! Cannot find package file: ${files[${a}]}"
        fi
        if [ ! -d ${dirs[${a}]} ];then  #invalid destination path
            echo -e "\tWARNING! Directory not found: ${dirs[${a}]}"
        fi
        if [ $a -eq 1 ];then  #is non-essential
            echo -e "\t*Icon will not be installed."
            icon=0
        fi
    fi
done


## CHECK SHARED LIBRARIES.
echo -e "\nChecking shared libraries..."
command -v ldconfig &> /dev/null;b=$?  #exit status

if command -v objdump &> /dev/null;then  #objdump is available
    libs=($(objdump -p ${files[0]} | grep NEEDED | awk '{print $2}'))  #required libs
    c=${#libs[@]}  #number of
    for ((d=0;d<$c;d++));do
        if [[ ! ${libs[${d}]} =~ ".so" ]];then  #unexpected result
            unk=1
        fi
    done
    if [[ $unk -eq 0 && $b -eq 0 ]];then  #ldconfig is available
        for ((d=0;d<$c;d++));do
            if ldconfig -p | grep ${libs[${d}]} &> /dev/null;then  #.so found
                echo -e "\t found: ${libs[${d}]}"
            else
                echo -e "\tWARNING! Library not found: ${libs[${d}]}"
                warning=1
            fi
        done
    elif [ $unk -eq 0 ];then  #must be: ldconfig not available
        echo -e "\tWARNING! Unable to check libraries (utility unavailable: ldconfig)."
        echo -e "\t*Before installing $app, ensure the following are available:"
        warning=1
        for ((d=0;d<$c;d++));do
            echo -e "\t\t${libs[${d}]}"
        done
    else #must be: unk=1
        echo -e "\tWARNING! (unknown issue) - Unable to check libraries."
        warning=1
    fi
else  #objdump not available
    warning=1
    if [ $b -ne 0 ];then
        echo -e "\tWARNING! Unable to check libraries (utilities unavailable: objdump, ldconfig)"
    else echo -e "\tWARNING! Unable to check libraries (utility unavailable: objdump)"
    fi
fi


## SUMMARIZE RESULTS & PROMPT TO INSTALL.
if [ $warning -eq 0 ];then
    echo -e "\n\nNo issues were detected."
else echo -e "\n\nIssue(s) detected! Please review warning messages above before proceeding."
fi
if [ $installed -eq 1 ];then
    echo -e "A pre-existing installation was found."
    echo -n "Remove and re-install $app? (y/n) "
else echo -n "Install $app? (y/n) "
fi
read  #prompt
until [[ $REPLY = 'y' || $REPLY = 'n' ]];do
    echo -n "Enter y or n: "
    read  #re-prompt
done
if [ $REPLY = 'n' ];then
    echo -e "\n$app was not installed."
    echo -e "Exiting...\n"
    exit
fi


##########################################################################################
#                                      Installation                                      #
##########################################################################################

warning=0  #re-set


## REMOVE PREVIOUS INSTALLATION.
if [ $installed -eq 1 ];then
    echo -e "\nRemoving current installation..."
    for ((a=0;a<4;a++));do
        if [ -f ${result[${a}]} ];then  #file found
            if sudo rm ${result[${a}]} && [ ! -f ${result[${a}]} ];then  #remove (success & re-confirm)
                echo -e "\t removed: ${result[${a}]}"
            else
                echo -e "\tWARNING! Could not remove file: ${result[${a}]}"
                warning=1
            fi
        else echo -e "\t not found: ${result[${a}]}"  #inform but proceed
        fi
    done
fi


## INSTALL PACKAGE FILES.
if [ $installed -eq 1 ];then
    echo -e "\nRe-installing package files..."
else echo -e "\nInstalling package files..."
fi
for ((a=0;a<4;a++));do
    if [[ $a -eq 2 && $icon -eq 0 ]];then  #skip icon if prior warning
        echo -e "\t did NOT install: ${files[${a}]}"
    else
        if sudo cp ${files[${a}]} ${result[${a}]} &> /dev/null && [ -f ${result[${a}]} ];then  #install file (success & re-confirm)
            echo -e "\t installed: ${result[${a}]}"
            if [[ $a -eq 0 || $a -eq 1 ]];then  #is program or uninstaller
                if sudo chmod +x ${result[${a}]} &> /dev/null && [ -x ${result[${a}]} ];then  #permission to execute (success & re-confirm)
                    echo -e "\t permission to execute: ${result[${a}]}"
                else
                    echo -e "\tWARNING! Could not grant permission to execute: ${result[${a}]}"
                    warning=1
                fi
            elif [[ $a -eq 3 && $icon -eq 0 ]];then  #is desktop & skipped icon
                if [ $(grep Icon= ${result[${a}]}) ];then  #file contains
                    c=$(sed -n '/Icon=/=' ${result[${a}]})  #line number
                    if sudo sed -i "${c}d" ${result[${a}]} &> /dev/null && [ ! $(grep Icon= ${result[${a}]}) ];then  #remove (success & re-confirm)
                        echo -e "\tREMOVED icon reference from: ${result[${a}]}"
                    fi
                fi
            fi
        else
            echo -e "\tWARNING! Could not install: ${result[${a}]}"
            warning=1
        fi
    fi
done


## UPDATE GTK-4 ICON CACHE.
if [ $icon -eq 1 ];then
    echo -e -n "\nUpdating GTK-4 icon cache...\n\t"
    b='gtk4-update-icon-cache'
    if command -v $b &> /dev/null;then  #utility available
        sudo $b /usr/share/icons/hicolor  #update icon-theme.cache (display output)
    else
        echo "WARNING! Unsuccessful (utility unavailable: $b)"
        warning=1
    fi
fi


## DISPLAY POST-INSTALLATION MESSAGE.
if [ $warning -eq 1 ];then
    echo -e "\n\n\nIssue(s) occured during installation. Please review the warning messages above.\n"
else
    echo -e "\n\n\nInstallation is complete!"
    echo -e "\nIf all is well, the following files are no longer needed and may be removed:"
    for ((a=0;a<5;a++));do
        echo -e "\t$(pwd)/${files[${a}]}"
    done
    echo -e "\nPlease Ensure that your system is up to date via your package manager."
    echo -e "\nReboot is recommended before launching $app.\n"
fi


#end
