# jfind

<img width="1037" alt="image" src="https://user-images.githubusercontent.com/83528263/211302575-315d669c-2552-4213-a3a1-071fde7995fe.png">

### Neovim Plugin
You can use jfind as a neovim plugin. You can find the repo [here](https://github.com/jake-stewart/jfind.nvim).

### Install Instructions

#### brew

    brew install technicalpickles/technicalpickles/jfind

#### build from source

    cmake -S . -B build
    cd build
    sudo make install

## Example config (`~/.config/jfind/config.json`)
```json
{
    "selector": "",
    "active_selector": "",
    "history_limit": 20,
    "matcher": "fuzzy",
    "case_mode": "insensitive",
    "window_style": "merged",
    "style": {
        "active_item": {
            "fg": "bright_white",
            "bg": 233,
            "attr": ["bold"]
        },
        "hint": {
            "fg": 240
        },
        "border": {
            "fg": 236
        },
        "active_row": {
            "bg": 233
        },
        "preview_line": {
            "bg": 233
        },
        "active_selector": {
            "bg": 233
        },
        "active_hint": {
            "fg": "bright_white",
            "bg": 233
        }
    }
}
```
