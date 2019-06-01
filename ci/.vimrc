" =================================================================================================
"                            Boubacar DIENE<boubacar.diene@gmail.com>
" =================================================================================================
"                       Vim  + Ctags + CtrlP + NERDTree + Vim-Clang-Format
" -------------------------------------------------------------------------------------------------
"
" 0- Install pathogen
"    $ mkdir -p ~/.vim/autoload ~/.vim/bundle
"    $ curl -LSso ~/.vim/autoload/pathogen.vim https://tpo.pe/pathogen.vim
"
" 1- Install ctags
"    $ sudo apt install ctags
"
"    $ git clone https://github.com/BoubacarDiene/mmstreamer.git
"    $ cd mmstreamer/
"    $ ctags -R .
"    $ vim src/video/Video.c
"
" 2- Install CtrlP
"    $ git clone https://github.com/kien/ctrlp.vim.git ~/.vim/bundle/ctrlp.vim
"    $ echo "set runtimepath^=~/.vim/bundle/ctrlp.vim" >> ~/.vimrc
"
"    $ vim
"    $ :helptags ~/.vim/bundle/ctrlp.vim/doc
"    $ :q!
"
" 3- Install nerdtree
"    $ git clone https://github.com/scrooloose/nerdtree.git ~/.vim/bundle/nerdtree
"    $ :helptags ~/.vim/bundle/nerdtree/doc/
"
" 4- Install vim-qml
"    $ git clone https://github.com/peterhoeg/vim-qml.git ~/.vim/bundle/vim-qml
"
" 5- Install vim-clang-format (https://github.com/rhysd/vim-clang-format)
"
"    5.1- Install clang-format
"    $ sudo apt install clang-format
"    $ clang-format -dump-config
"
"    5.2- Install vim-operator-user
"    $ git clone https://github.com/kana/vim-operator-user.git ~/.vim/bundle/vim-operator-user
"
"    5.3- Install vim-clang-format
"    $ git clone https://github.com/rhysd/vim-clang-format.git ~/.vim/bundle/vim-clang-format
"
" ------------------------------------------------------------------------------------------------
"
" - :sp                     --> Split horizontally
" - :vsp                    --> Split vertically
" - :e <file>               --> Replace current content with <file>
" - :help <keyword>         --> Keyword can be a command (sp, vsp, tag, ...)
"
" - :tag <keyword>          --> Search for an existing tag
" - :ts                     --> show tags list
" - Ctrl+T                  --> Stop ctags search
" - Ctrl+AltGR+]            --> (Ctrl-]). Cursor in a function then this will take you there
"
" - Ctrl+P                  --> Search file or directory using CtrlP plugin
" - Ctrl+O                  --> Leave CtrlP
"
" - vim <dir>               --> Open NERDTree and browse <dir> content
" - :NERDTree <dir>         --> To browse <dir> content (CtrlP possible in opened view)
" - Ctrl+O                  --> Back to previous file
" - q                       --> Quit NERDTree
" - :NERDTreeFind           --> Find the currently active file in the filesystem
" - :NERDTreeRefreshRoot    --> Refresh root
"
" - Ctrl+W                  --> Switch between windows (x2 to make it work?)
" - :tabe %                 --> Pop out a new tab temporarily
" - :q                      --> Back to the splitted windows
"
" - v                       --> Enter VISUAL mode (Esc to go back to normal mode)
" - V                       --> Enter VISUAL LINE mode (Esc to go back to normal mode)
" - y                       --> Copy selected text (still in visual mode)
" - p                       --> Paste selected text (still in visual mode)
" - <N>j                    --> Select the <N>th lines in VISUAL mode
"
" - :sh                     --> Start the shell
" - exit                    --> Back to vim (from the shell)
"
" - :set <cmd>!             --> Undo cmd (example: set number! will remove numbers)
"
" - :u                      --> Undo
" - Ctrl+R                  --> Redo
"
" - G                       --> Go to end of file (<N>G possible with N >= 1)
" - gg                      --> Go to beginning of file
" - $                       --> Go to end of line
"
" - :ClangFormat            --> In NORMAL mode: Format the whole code
"                               In VISUAL mode: Format the selected code
"
" - :ClangFormatAutoEnable  --> Enable autoformat in current buffer
" - :ClangFormatAutoDisable --> Disable autoformat in current buffer
"
" =================================================================================================
" pathogen.vim makes it super easy to install plugins and runtime files in their
" own private directories (See https://github.com/tpope/vim-pathogen)
" =================================================================================================
execute pathogen#infect()
syntax on
filetype plugin indent on

" =================================================================================================
" Indentation: Tab replaced with 4 spaces
" =================================================================================================
set tabstop=4 softtabstop=0 expandtab shiftwidth=4 smarttab

" =================================================================================================
" - Show line numbers
" - Show column numbers
" - Ignore case
" - Highlight search
" - Show match (brackets, ...)
" - Show (partial) command in the last line of the screen
" - Minimal number of screen lines to keep above and below the cursor
" - Line position tracking
" - Remove the underline
" - Line number in blue
" =================================================================================================
set number
set ruler
set ignorecase
set hlsearch
set showmatch
set showcmd
set scrolloff=5
set cursorline
highlight clear CursorLine
highlight CursorLineNR ctermbg=blue

" =================================================================================================
" CtrlP: See http://kien.github.io/ctrlp.vim/#installation
" =================================================================================================
set runtimepath^=~/.vim/bundle/ctrlp.vim
