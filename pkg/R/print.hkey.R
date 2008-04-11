`print.hkey` <-
function (x, ...) 
{
    cat(paste("Win32 registry node: ", attr(x, "key"), "\nUse names() to list subkeys and data, $, [], [[]] to extract them.\n", 
        sep = ""))
}
