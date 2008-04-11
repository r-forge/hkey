`names.hkey` <-
function (x) 
{
    c(if (length(attr(x, "children")) > 0) {
        paste(attr(x, "children"), hkey.subkey.suffix, sep = "")
    } else {
        NULL
    }, ls(attr(x, "values"), all.names = TRUE))
}
