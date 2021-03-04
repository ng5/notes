# shellcheck shell=bash
function checkSource() {
  local file=$1
  [ -f "$file" ] && source "$file"
}
export ZSH=~/.oh-my-zsh
export PROMPT_EOL_MARK=''
# shellcheck disable=SC2034
ZSH_THEME="robbyrussell"
# shellcheck disable=SC2034
plugins=(zsh-syntax-highlighting zsh-autosuggestions docker docker-compose git adb fzf rsync)
checkSource $ZSH/oh-my-zsh.sh
checkSource ~/.fzf.zsh
checkSource ~/miniconda3/etc/profile.d/conda.sh
checkSource ~/.cargo/env

alias 1="source ~/.zshrc"
export FZF_DEFAULT_OPTS="-i --ansi --bind alt-k:preview-page-up,alt-j:preview-page-down --height 100% --preview 'batcat --theme=GitHub --color=always --style=header,grid --line-range :300 {}' "
export FZF_DEFAULT_COMMAND="rg --files "

# shellcheck disable=SC2154
export PROMPT="%{$fg_bold[red]%}%n@%m ${PROMPT}"
export PATH=$PATH:/usr/local/go/bin:~/miniconda3/bin:~/GOROOT/go/bin
export EDITOR=vim
unsetopt AUTO_CD
unsetopt nomatch
