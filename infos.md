# autosave in neovim
set autowriteall
au InsertLeavePre,TextChanged,TextChangedP * if &modifiable && !&readonly | silent! update | endif
