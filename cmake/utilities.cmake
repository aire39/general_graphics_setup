# utility functions

function(colored_message color_code message_text)
    string(ASCII 27 esc)
    message(STATUS "${esc}[${color_code}m${message_text}${esc}[0m")
endfunction()