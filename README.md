# jfind

`jfind` is a better version of [fzf](https://github.com/junegunn/fzf).

 * Faster loading
 * Faster sorting
 * Less memory usage
 * Better sorting results


<img width="1037" alt="image" src="https://user-images.githubusercontent.com/83528263/211302575-315d669c-2552-4213-a3a1-071fde7995fe.png">

### Inspiration

`jfind` was originally written before `fzf` version [0.32.0](https://github.com/junegunn/fzf/releases/tag/0.32.0).
Prior to this verison, `fzf` had a terrible scoring algorithm.
Instead of switching back to `fzf`, I continue to use `jfind` due to the better performance and nice aesthetic.

### Install Instructions

    cmake -S . -B build
    cd build
    sudo make install

### Uninstall Instructions

    sudo make uninstall

or

    sudo rm /usr/local/bin/jfind
