_conduit() {
    local cur prev commands
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"

    commands="topics info echo hz record flow"

    if [[ ${COMP_CWORD} -eq 1 ]]; then
        COMPREPLY=($(compgen -W "${commands}" -- "${cur}"))
        return 0
    fi

    case "${COMP_WORDS[1]}" in
        flow)
            if [[ ${COMP_CWORD} -eq 2 ]]; then
                local flows
                flows="$(conduit flow --complete 2>/dev/null)"
                COMPREPLY=($(compgen -W "${flows}" -- "${cur}"))
            fi
            ;;
        info|echo|hz)
            if [[ ${COMP_CWORD} -eq 2 ]]; then
                local topics
                topics="$(conduit topics 2>/dev/null)"
                COMPREPLY=($(compgen -W "${topics}" -- "${cur}"))
            fi
            ;;
    esac
}

complete -F _conduit conduit
