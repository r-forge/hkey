"[[<-.hkey" <-
function (x, i, use.value=FALSE, value) 
{
  if (i == "")
    i <- hkey.default.valname
  do.delete <- is.null(value)
  is.subkey <- is.list(value) && !use.value
  is.existing.subkey <- i %in% attr(x, "children") && !use.value
  is.existing.value <- (!is.existing.subkey) && (i %in% names(x))
  if (!do.delete && is.existing.subkey && !is.subkey) 
    warning("You are assigning a data item to the subkey named ", 
            i, ".\nI will delete the original data item.")
  if (!do.delete && is.existing.value && is.subkey) 
    warning("You are assigning a subkey structure to the data item named ", 
            i, ".\nI will delete the original subkey.")
  if (is.existing.subkey) 
    .Call("delete_subnode", x, i)
  else if (is.existing.value) 
    .Call("delete_value", x, if (i == hkey.default.valname) "" else i)
  if (!is.null(value)) {
    if (is.subkey) {
      hkey.new.key(x, i, value)
    }
    else {
      hkey.assign.value(x, i, value)
    }
  }
  hkey.fillin.hkey(x)
}
