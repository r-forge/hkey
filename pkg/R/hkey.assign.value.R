`hkey.assign.value` <-
function (x, valname, val) 
{
    if (valname == hkey.default.valname) 
        valname <- ""
    if (is.character(val)) {
        if (length(val) == 1) {
            if (length(grep(hkey.hkey.regex, val, perl = TRUE)) > 
                0) 
                type <- hkey.types$REG_LINK
            else if (length(grep(hkey.expand.regex, val, perl = TRUE)) > 
                0) 
                type <- hkey.types$REG_EXPAND_SZ
            else type <- hkey.types$REG_SZ
        }
        else {
            type <- hkey.types$REG_MULTI_SZ
        }
    }
    else if (is.numeric(val)) {
        val <- as.integer(val)
        if (length(val) == 1) 
            type <- hkey.types$REG_DWORD
        else if (length(val) == 2) 
            type <- hkey.types$REG_QWORD
        else stop("Cannot set key value from a numeric vector of length greater than 2")
    }
    else if (inherits(val, "raw")) {
        type <- hkey.types$REG_BIN
    }
    else {
        stop("I don't know how to assign a registry node value from an object of class ", 
            class(val), " and length ", length(val))
    }
    if (valname == "" && type != hkey.types$REG_SZ) 
        stop("The default (unnamed) item of a node can only be assigned a character string")
    .Call("set_key_value", x, valname, as.integer(type), val)
}
