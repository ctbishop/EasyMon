## EasyMon
System monitor app for Linux operating systems - Written in C using GTK-4. 

<br/>

#### I. AUTHOR CONTACT  
  
&nbsp;&nbsp;&nbsp;&nbsp; C. Bishop - [contact me](https://casandrabishop.com/contact.html)  

<br/>

#### II. DISCLAIMER  
  
    Although this archive/repository was assembled with the best intentions,  
    neither this archive/repository nor it contents are guaranteed to function  
    as intended or to produce any particular result. Thus, the possession and/or  
    application of this archive/repository and/or any its contents, including  
    the results of such, is done at your own risk. Please report bugs and other  
    issues either here on GitHub or directly via the author contact link above.  

<br/>

#### III. TABLE OF CONTENTS  
  
    1. EasyMon - About
    2. Archive Contents
    3. Installation Instructions
    4. Post-Installation
    5. Launching EasyMon
    6. Deinstallation

<br/>

#### 1. EASYMON - ABOUT  
   
    EasyMon is an uncomplicated system monitor, designed to be ideal for use  
    during resource-intensive tasks. The app features a small window that fits  
    nicely in the corner of the screen and a compact, color-coded display. By  
    default, a collapsed view (280x230 px) is shown containing overall CPU and  
    RAM usage. This may be expanded by the user to show additional details as  
    well as CPU frequencies; the window resizes accordingly.  
  
    At this time, all information is updated strictly once per second. Memory  
    display units are adjustable. Easymon is compatible with both light and  
    dark OS themes, and accommodates up to sixteen total cores, or eight with  
    hyperthreading enabled.  

<br/>

#### 2. ARCHIVE/REPOSITORY CONTENTS  

    This archive consists of a single directory "EasyMon" containing precisely  
    the following files, all executeables being initially deactivated (save for  
    the directory itself):  
  
  
    File:                                    Description:  
  
    easymon.c                                source code for EasyMon program
    easymon-icon.svg                         shortcut/sidebar icon image  
    com.gmail.ctbishop34.EasyMon.desktop     desktop file  
    easymon-install-reinstall.sh             installation script  
    easymon-uninstall.sh                     deinstallation script  
    README.txt                               (this document)  
  
  
    If this is not the case, you are advised to remove the package entirely  
    from your system and contact the originator.  

<br/>

#### 3. INSTALLATION INSTRUCTIONS  
  
    Before before installing EasyMon, please ensure that you are using a Linux  
    operating system that supports GTK-4, and install the GTK-4 development library  
    if it is not already installed. You can do this via your package manager. e.g.,  
    if you use apt,  
  
    Update your list of available packages:  
        sudo apt update  
          
    Check availability and installation status:  
        apt list libgtk-4-dev  
          
    Install:  
        sudo apt install libgtk-4-dev  
  
  
    Once you have the GTK-4 development library, you will need to compile the source  
    code file for the EasyMon program, easymon.c. Download the EasyMon package, if  
    you haven't already. This includes all files in the main branch of the EasyMon  
    repository. Please download all files to a single directory named "EasyMon" which  
    contains no other files. Then,  
  
    Navigate to the directory. e.g.:  
        cd ~/Downloads/EasyMon  
          
    Compile, naming the output program "easymon" (without quotes):  
        gcc $( pkg-config --cflags gtk4 ) -o easymon easymon.c $( pkg-config --libs gtk4 )  
  
  
    Should you encounter any trouble compiling the program, the GTK-4 documentation  
    may be helpful and can be found at [https://docs.gtk.org/gtk4/](https://docs.gtk.org/gtk4/). Once compiled,  
    the source code is no longer needed. Now you are ready to install. From the  
    EasyMon directory,  
  
    Give the installer permission to execute:  
        chmod +x easymon-install-reinstall.sh  
  
    Launch the installer:  
        ./easymon-install-reinstall.sh  
  
  
    Once the installer has been launched, you will be prompted for your password  
    to continue. The installer will then do a pre-check of your system, reporting  
    any foreseen issues, then prompt you to either continue (install) or exit.  
  
    If you choose to continue, the installer will report the name and location of  
    files as they are installed, as well as other changes it made and any issues  
    that were detected. If you chose not to install or if errors occurred and you  
    need to re-install, you can re-run the installer again at any time.  
  
    Following successful installation, the EasyMon directory and its contents are  
    no longer needed and may be removed from your system.  

<br/>

#### 4. POST-INSTALLATION  
  
    Before launching EasyMon, it is a good idea to reboot your machine  
    as well as to ensure that your system is up to date.  
  
    You can update your system via your package manager.  
    e.g., if you use apt, enter:  
        sudo apt update  
  
    followed by:  
        sudo apt upgrade  

<br/>

#### 5. LAUNCHING EASYMON  
  
    Following installation, EasyMon can be launched from the desktop icon  
    as well as from the command line:  
        easymon  

<br/>

#### 6. DEINSTALLATION  
  
    The installation of EasyMon includes an uninstaller.  
    Should you decide to uninstall EasyMon, simply type into the command line:    
        easymon-uninstall  
  
  
    Once launched, the uninstaller will prompt for your password, then ask if you  
    would like to proceed. If you choose to continue, the uninstaller will find  
    and remove the installed files, reporting its progress. It will then pause  
    once more for input before removing itself.  

<br/>

(end of document)
