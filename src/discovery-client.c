int main
  (int argc,
  char *argv [])
{
#if 0
  init to undiscovered
  read last known pd address (optional)

  wait for a command
  if it's not config address
    if it's not for me
      ignore it
    else
      nak it (optional?)
  else /* it is config address */
    if it's not a discovery command nak it (optional?)
    else
      if it's start-discover do nothing, log it
      if it's discover then random backoff and respond
      if it's discovery-set then set address, ack, leave undiscovered mode, output config, stop
#endif
  return(0);
}

