# Raider <img align="right" alt="Number of downloads" src="https://img.shields.io/flathub/downloads/com.github.ADBeveridge.Raider?style=flat-square"> <img  align="right" alt="AUR" src="https://img.shields.io/aur/version/raider-file-shredder?style=flat-square"></a> <img align="right" alt="Fedora COPR build status" src="https://copr.fedorainfracloud.org/coprs/0xmrtt/raider/package/raider/status_image/last_build.png"/>

Raider is a shredding program built for the GNOME desktop. It uses its own internal shredding code instead of an external program.

## Download
<a href='https://beta.flathub.org/apps/details/com.github.ADBeveridge.Raider'><img width='240' alt='Download on Flathub' src='https://flathub.org/assets/badges/flathub-badge-en.png'/></a>

# Compiling
## GNOME Builder
* Install GNOME Builder. This should be in your distro's repositories. It is also available here: https://flathub.org/apps/org.gnome.Builder
* Download the source of this project from GitHub.
* Open the project in Builder, and after installing SDKs, click the Run command.
## Commandline

### Ubuntu 22.04 LTS

To build natively requires additional work due to Ubuntu 22.04 LTS not currently distributing libadwaita-1-4 and therefore certain UI elements are not available. There is a workaround for this, but it's not that appealing. This was tested in a GNOME box.

Install git.

* `sudo apt update && sudo apt install git -y`

Clone this repository to the desired location.

* `git clone https://github.com/Spice-Weasel/raider`

This project requires meson version >=0.63 to build, which is unavailable in the package repositories for Ubuntu currently, so you will need to either install using pip or downloading the release. Information on how to install this can be found here <https://mesonbuild.com/Getting-meson.html>

In addition to this, in order to build the project on Ubuntu 22.04 LTS requires building Gtk4 - so there are many dependencies to install.

* `sudo apt install python3-distutils libxml2-dev libgtk-4-dev libcurl4-openssl-dev libyaml-dev libzstd-dev gettext itstool xsltproc libappstream-dev appstream-util ninja-build`

Configure the project.

* `cd raider`
* `meson setup builddir`

As Ubuntu 22.04 LTS does not include libadwaita-1-4 in the package repos, the distribution does not ship with a typelib required to run it. I found the best solution was to download from a third-party source.

* ` wget https://download.opensuse.org/repositories/openSUSE:/Factory/standard/x86_64/typelib-1_0-Adw-1-1.4.0-1.2.x86_64.rpm`

Extract the file `Adw-1.typelib` to your home directory.

Check the file integrity:

* `sha256sum ~/Adw-1.typelib`

You should get the following output `80e587b67807bd788087fc84fee2f763c2fbb4a751ad38a275973314fdeca2c5` if not then you should not continue with the installation.

Once you have verified the file integrity, you should back-up the preinstalled version of this file.

* `sudo mv /usr/lib/x86_64-linux-gnu/girepository-1.0/Adw-1.typelib /usr/lib/x86_64-linux-gnu/girepository-1.0/Adw-1.typelib.bak`

You can then move the more recent version of the file from your home directory to the required location.

* `sudo mv ~/Adw-1.typelib /usr/libx86_64-linux-gnu/girepository-1.0/Adw-1.typelib`

Return to the project repo and build the project.

* `cd builddir`
* `ninja`

### Fedora
* `sudo dnf install meson gcc gtk4-devel libadwaita-devel desktop-file-utils python3-gobject`
* `meson setup builddir`
* `cd builddir`
* `ninja`
* `sudo meson install`

## Useful Links
*   GNOME Circle: <https://apps.gnome.org/Raider>
*   Arch Linux AUR: <https://aur.archlinux.org/packages/raider-file-shredder>
*   Fedora COPR: <https://copr.fedorainfracloud.org/coprs/0xmrtt/raider/package/raider/>
*   Build manifest: <https://github.com/flathub/com.github.ADBeveridge.Raider>
*   Report issues: <https://github.com/ADBeveridge/raider/issues/>
