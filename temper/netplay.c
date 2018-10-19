#include "common.h"

s32 receive_packet(control_buffer_entry_struct *remote_controls);

#ifdef WIN32_BUILD
  //typedef struct sockaddr sockaddr_type;
  typedef struct sockaddr_in sockaddr_type;
#else
  typedef struct sockaddr_in sockaddr_type;
#endif

static void print_netplay_error(char *str)
{
#ifdef WIN32_BUILD
  typedef struct
  {
    s32 number;
    char *description;
  } error_description_pair_struct;

  u32 search_pair;
  u32 error_number = WSAGetLastError();

  error_description_pair_struct error_description_pairs[] =
  {
    { 0,                  "No error"                            },
    { WSAEINTR,           "Interrupted system call"             },
    { WSAEBADF,           "Bad file number"                     },
    { WSAEACCES,          "Permission denied"                   },
    { WSAEFAULT,          "Bad address"                         },
    { WSAEINVAL,          "Invalid argument"                    },
    { WSAEMFILE,          "Too many open sockets"               },
    { WSAEWOULDBLOCK,     "Operation would block"               },
    { WSAEINPROGRESS,     "Operation now in progress"           },
    { WSAEALREADY,        "Operation already in progress"       },
    { WSAENOTSOCK,        "Socket operation on non-socket"      },
    { WSAEDESTADDRREQ,    "Destination address required"        },
    { WSAEMSGSIZE,        "Message too long"                    },
    { WSAEPROTOTYPE,      "Protocol wrong type for socket"      },
    { WSAENOPROTOOPT,     "Bad protocol option"                 },
    { WSAEPROTONOSUPPORT, "Protocol not supported"              },
    { WSAESOCKTNOSUPPORT, "Socket type not supported"           },
    { WSAEOPNOTSUPP,      "Operation not supported on socket"   },
    { WSAEPFNOSUPPORT,    "Protocol family not supported"       },
    { WSAEAFNOSUPPORT,    "Address family not supported"        },
    { WSAEADDRINUSE,      "Address already in use"              },
    { WSAEADDRNOTAVAIL,   "Can't assign requested address"      },
    { WSAENETDOWN,        "Network is down"                     },
    { WSAENETUNREACH,     "Network is unreachable"              },
    { WSAENETRESET,       "Net connection reset"                },
    { WSAECONNABORTED,    "Software caused connection abort"    },
    { WSAECONNRESET,      "Connection reset by peer"            },
    { WSAENOBUFS,         "No buffer space available"           },
    { WSAEISCONN,         "Socket is already connected"         },
    { WSAENOTCONN,        "Socket is not connected"             },
    { WSAESHUTDOWN,       "Can't send after socket shutdown"    },
    { WSAETOOMANYREFS,    "Too many references, can't splice"   },
    { WSAETIMEDOUT,       "Connection timed out"                },
    { WSAECONNREFUSED,    "Connection refused"                  },
    { WSAELOOP,           "Too many levels of symbolic links"   },
    { WSAENAMETOOLONG,    "File name too long"                  },
    { WSAEHOSTDOWN,       "Host is down"                        },
    { WSAEHOSTUNREACH,    "No route to host"                    },
    { WSAENOTEMPTY,       "Directory not empty"                 },
    { WSAEPROCLIM,        "Too many processes"                  },
    { WSAEUSERS,          "Too many users"                      },
    { WSAEDQUOT,          "Disc quota exceeded"                 },
    { WSAESTALE,          "Stale NFS file handle"               },
    { WSAEREMOTE,         "Too many levels of remote in path"   },
    { WSASYSNOTREADY,     "Network system is unavailable"       },
    { WSAVERNOTSUPPORTED, "Winsock version out of range"        },
    { WSANOTINITIALISED,  "WSAStartup not yet called"           },
    { WSAEDISCON,         "Graceful shutdown in progress"       },
    { WSAHOST_NOT_FOUND,  "Host not found"                      },
    { WSANO_DATA,         "No host data of that type was found" },
    { -1,                 "END"                                 }
  };

  for(search_pair = 0; search_pair != -1; search_pair++)
  {
    if(error_description_pairs[search_pair].number == error_number)
    {
      status_message("Netplay error: %s: %s\n", str,
       error_description_pairs[search_pair].description);
      break;
    }
  }

  if(search_pair == -1)
  {
    status_message("Netplay error: %s: unknown\n", str);
  }
#else
  status_message("Netplay error: %s: %s\n", str, strerror(errno));
#endif
}

netplay_struct netplay;

void flush_receive(void)
{
  control_buffer_entry_struct remote_controls;
  while(receive_packet(&remote_controls) >= 0);
}

void reset_control_buffer(control_buffer_struct *control_buffer)
{
  control_buffer->write_position = 0;
  control_buffer->last_written = NULL;

  memset(control_buffer->entries, 0,
   sizeof(control_buffer_entry_struct) * CONTROL_BUFFER_SIZE);
}

void queue_controls(control_buffer_struct *control_buffer,
 u32 frame_number, u32 controls)
{
  u32 write_position = control_buffer->write_position;
  control_buffer_entry_struct *control_buffer_entry =
   &(control_buffer->entries[write_position]);

  control_buffer_entry->controls = controls;
  control_buffer_entry->frame_number = frame_number;
  control_buffer->last_written = control_buffer_entry;
  control_buffer->write_position = (write_position + 1) % CONTROL_BUFFER_SIZE;
}


// TODO: This can be done better, you can find out sooner if you definitely
// won't find the frame.

control_buffer_entry_struct
 *find_controls_at_frame_number(control_buffer_struct *control_buffer,
 u32 frame_number)
{
  control_buffer_entry_struct *new_controls;
  u32 base_write_position = control_buffer->write_position;
  u32 seek_write_position =
   (control_buffer->write_position - 1) % CONTROL_BUFFER_SIZE;

  while(seek_write_position != base_write_position)
  {
    new_controls = &(control_buffer->entries[seek_write_position]);
    if(new_controls->frame_number == frame_number)
      return new_controls;

    seek_write_position = (seek_write_position - 1) % CONTROL_BUFFER_SIZE;
  }
  return NULL;
}


void netplay_disconnect(void)
{
  close(netplay.socket_handle);
  audio_unpause();

  netplay.pause = 0;
  netplay.active = 0;
  netplay.can_send = 0;
}

void invalidate_send_buffer(void)
{
  netplay.send_control_packets_buffered = 0;
  netplay.send_buffer_ptr = netplay.send_buffer;
  netplay.send_buffer_bytes = 0;
}

void netplay_queue_latency_filler(void)
{
  u32 back_frame;
  u32 i;

  // Have a few dummy frames going into the past before things started to
  // give us some initial latency to work with.

  for(i = 0, back_frame = (0 - netplay.frame_latency) & 0xFFFF;
   i <= netplay.frame_latency; i++, back_frame++)
  {
    queue_controls(&netplay.control_buffer_local, back_frame, 0xFFFF);
    queue_controls(&netplay.control_buffer_remote, back_frame, 0xFFFF);
  }
}

void netplay_reset_buffers(void)
{
  invalidate_send_buffer();

  reset_control_buffer(&(netplay.control_buffer_local));
  reset_control_buffer(&(netplay.control_buffer_remote));

  netplay_queue_latency_filler();
  netplay.frame_number = 0;
}


s32 netplay_connect_server(void)
{
  sockaddr_type socket_address;

  netplay.frame_latency = config.netplay_server_frame_latency;

  if(config.netplay_username[0] == 0)
    strcpy(config.netplay_username, "Player 1");

  netplay.socket_handle = socket(AF_INET, SOCK_STREAM, 0);
  netplay.active = 1;

  if(netplay.socket_handle < 0)
  {
    print_netplay_error("socket");
    netplay_disconnect();
    return -1;
  }

  socket_address.sin_family = AF_INET;
  socket_address.sin_port = htons(config.netplay_port);
  socket_address.sin_addr.s_addr = htonl(INADDR_ANY);

  if(bind(netplay.socket_handle, &(socket_address),
   sizeof(sockaddr_type)) < 0)
  {
    print_netplay_error("bind");
    netplay_disconnect();
    return -1;
  }

  if(listen(netplay.socket_handle, 5) < 0)
  {
    print_netplay_error("listen");
    netplay_disconnect();
    return -1;
  }

  return netplay.socket_handle;
}

s32 netplay_connect_client(void)
{
  sockaddr_type server_address;
  control_buffer_entry_struct remote_controls;
  u32 tries;

  if(config.netplay_username[0] == 0)
    strcpy(config.netplay_username, "Player 2");

  netplay.socket_handle = socket(AF_INET, SOCK_STREAM, 0);
  netplay.active = 1;
  netplay.can_send = 1;

  if(netplay.socket_handle < 0)
  {
    print_netplay_error("socket");
    netplay_disconnect();
    return -1;
  }

  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(config.netplay_port);
  server_address.sin_addr.s_addr = htonl(config.netplay_ip);

  status_message("Trying to connect to IP %d.%d.%d.%d\n",
   config.netplay_ip >> 24, (config.netplay_ip >> 16) & 0xFF,
   (config.netplay_ip >> 8) & 0xFF, config.netplay_ip & 0xFF);

  if(connect(netplay.socket_handle, &server_address,
   sizeof(sockaddr_type)) < 0)
  {
    print_netplay_error("connect");
    netplay_disconnect();
    return -1;
  }  

  // Now we need to get the frame latency, give it up to 50ms * 200 (10s)
  for(tries = 0; tries < 50 * 200; tries++)
  {    
    if(receive_packet(&remote_controls) == NETPLAY_PACKET_TYPE_SET_LATENCY)
    {
      netplay.frame_latency = netplay.frame_latency_update_to;
      printf("Got latency %d\n", netplay.frame_latency);  
      return netplay.socket_handle;
    }

    delay_us(50000);
  }

  status_message_raw("Err: could not get latency setting");
  netplay_disconnect();

  return -1;
}

void disable_blocking(void)
{
#ifdef WIN32_BUILD
  u_long ioctl_mode = 1;
  ioctlsocket(netplay.socket_handle, FIONBIO, &ioctl_mode);
#else
  u32 fcntl_flags = fcntl(netplay.socket_handle, F_GETFL);
  fcntl(netplay.socket_handle, F_SETFL, fcntl_flags | O_NONBLOCK);
#endif
}

s32 server_wait_for_connection(void)
{
  sockaddr_type client_address;
  u32 client_address_length = sizeof(sockaddr_type);

  s32 accept_handle =
   accept(netplay.socket_handle, &client_address, &client_address_length);

  if(accept_handle >= 0)
  {
    u32 accept_ip = client_address.sin_addr.s_addr;
    status_message("Received connection from %d.%d.%d.%d",
     accept_ip & 0xFF, (accept_ip >> 8) & 0xFF, (accept_ip >> 16) & 0xFF,
     accept_ip >> 24);

    close(netplay.socket_handle);
    netplay.socket_handle = accept_handle;
    disable_blocking();
  }

  return accept_handle;
}

u32 netplay_ip_string_value(const char *ip_string)
{
  u32 d0, d1, d2, d3;

  sscanf(ip_string, "%d.%d.%d.%d", &d0, &d1, &d2, &d3);
  return d3 | (d2 << 8) | (d1 << 16) | (d0 << 24);
}

void flush_send_buffer(void)
{
  s32 sent_bytes;

  u8 *send_buffer = netplay.send_buffer;
  u32 length = netplay.send_buffer_bytes;

  netplay.send_control_packets_buffered = 0;
  netplay.send_buffer_ptr = netplay.send_buffer;
  netplay.send_buffer_bytes = 0;

  while(1)
  {
    sent_bytes = send(netplay.socket_handle, send_buffer, length, 0);

    if(sent_bytes == length)
      break;

    if(sent_bytes >= 0)
    {
      send_buffer += sent_bytes;
      length -= sent_bytes;
    }
  }
}

void send_packet(u8 *send_buffer, u32 length)
{
  if(send_buffer[0] == NETPLAY_PACKET_TYPE_CONTROLS)
    (netplay.send_control_packets_buffered)++;

  netplay.send_buffer_bytes += length;
  memcpy(netplay.send_buffer_ptr, send_buffer, length);
  netplay.send_buffer_ptr += length;

  if(netplay.send_control_packets_buffered > netplay.send_control_buffer_limit)
    flush_send_buffer();
}

void send_controls(u32 controls, u32 frame_number)
{  
  u8 send_buffer[5] =
  {
    NETPLAY_PACKET_TYPE_CONTROLS, frame_number,
    frame_number >> 8, controls, controls >> 8
  };

  send_packet(send_buffer, 5);
}

void send_talk_message(char *message)
{
  u32 message_length = strlen(message);
  u8 send_buffer[message_length + 3];

  send_buffer[0] = NETPLAY_PACKET_TYPE_MESSAGE;
  send_buffer[1] = message_length;
  strcpy((char *)send_buffer + 2, message);
  send_packet(send_buffer, message_length + 3);
}

void send_latency_report(u32 latency_value, u32 stalls)
{
  u8 send_buffer[4] =
  {
    NETPLAY_PACKET_TYPE_LATENCY_REPORT, latency_value,
     latency_value >> 8, stalls
  };

  send_packet(send_buffer, 4);
}

void send_set_latency(u32 set_frame, u32 set_value)
{
  u8 send_buffer[4] =
  {
    NETPLAY_PACKET_TYPE_SET_LATENCY, set_frame,
     set_frame >> 8, set_value
  };

  send_packet(send_buffer, 4);
}

void send_netplay_pause(void)
{
  u8 send_buffer[1] = { NETPLAY_PACKET_TYPE_PAUSE };
  send_packet(send_buffer, 1);
  flush_send_buffer();
}

void send_netplay_unpause(void)
{
  u8 send_buffer[1] = { NETPLAY_PACKET_TYPE_UNPAUSE };
  send_packet(send_buffer, 1);
  flush_send_buffer();
}

void send_savestate_start(u32 savestate_length)
{
  u8 send_buffer[4] =
  {
    NETPLAY_PACKET_TYPE_SAVESTATE_START,
    savestate_length, savestate_length >> 8, savestate_length >> 16
  };

  send_packet(send_buffer, 4);
  flush_send_buffer();
}

void send_savestate_block(u8 *block_buffer,
 u32 block_length, u32 block_number)
{
  u8 send_buffer[block_length + 5];

  send_buffer[0] = NETPLAY_PACKET_TYPE_SAVESTATE_BLOCK;
  send_buffer[1] = block_length;
  send_buffer[2] = block_length >> 8;
  send_buffer[3] = block_number;
  send_buffer[4] = block_number >> 8;

  memcpy(send_buffer + 5, block_buffer, block_length);

  send_packet(send_buffer, block_length + 5);
  flush_send_buffer();
}

void send_savestate_block_confirm(u32 block_number)
{
  u8 send_buffer[3] =
  {
    NETPLAY_PACKET_TYPE_SAVESTATE_BLOCK_CONFIRM,
    block_number, block_number >> 8
  };

  send_packet(send_buffer, 3);
  flush_send_buffer();
}

#ifdef WIN32_BUILD

#define receive_packet_data_unavailable(_receive_bytes)                        \
 ((_receive_bytes == 0) || (WSAGetLastError() == WSAEWOULDBLOCK))              \

#else

#define receive_packet_data_unavailable(_receive_bytes)                        \
 ((_receive_bytes == 0) || (errno == EAGAIN))                                  \

#endif

#define receive_packet_ensure_available(size)                                  \
  while(receive_bytes < size)                                                  \
  {                                                                            \
    s32 additional_receive_bytes = recv(netplay.socket_handle, receive_buffer  \
     + receive_bytes, RECEIVE_BUFFER_LENGTH - receive_bytes, 0);               \
                                                                               \
    if(receive_packet_data_unavailable(additional_receive_bytes))              \
    {                                                                          \
      delay_us(1000);                                                          \
      continue;                                                                \
    }                                                                          \
                                                                               \
    if(additional_receive_bytes < 0)                                           \
    {                                                                          \
      print_netplay_error("receive");                                          \
      return -2;                                                               \
    }                                                                          \
    receive_bytes += additional_receive_bytes;                                 \
  }                                                                            \

s32 receive_packet(control_buffer_entry_struct *remote_controls)
{
  s32 receive_bytes;  
  u32 message_type;

  u8 *receive_buffer;

  if(netplay.receive_bytes_remaining > 0)
  {
    receive_bytes = netplay.receive_bytes_remaining;
  }
  else
  {
    netplay.receive_buffer_ptr = netplay.receive_buffer;

    receive_bytes = recv(netplay.socket_handle, netplay.receive_buffer,
     RECEIVE_BUFFER_LENGTH, 0);

    if(receive_bytes <= 0)
    {
      if(receive_packet_data_unavailable(receive_bytes))
        return -1;

      print_netplay_error("receive");
      return -2;
    }
  }
  receive_buffer = netplay.receive_buffer_ptr;

  message_type = receive_buffer[0];
  switch(message_type)
  {
    case NETPLAY_PACKET_TYPE_CONTROLS:
      receive_packet_ensure_available(5);

      remote_controls->frame_number = 
       receive_buffer[1] | (receive_buffer[2] << 8);
       remote_controls->controls = receive_buffer[3] | (receive_buffer[4] << 8);
      receive_bytes -= 5;
      receive_buffer += 5;
      break;

    case NETPLAY_PACKET_TYPE_MESSAGE:
    { 
      receive_packet_ensure_available(2);
      u32 packet_length = receive_buffer[1] + 3;

      receive_packet_ensure_available(packet_length);

      status_message_raw((char *)receive_buffer + 2); 
      if((packet_length - 3) > SCREEN_WIDTH_NARROW_CHARS)
      {
        status_message_raw((char *)receive_buffer + 2 +
         SCREEN_WIDTH_NARROW_CHARS);
      }

      receive_bytes -= packet_length;
      receive_buffer += packet_length;
      break;
    }

    case NETPLAY_PACKET_TYPE_LATENCY_REPORT:
      receive_packet_ensure_available(4);

      netplay.remote_latency_report =
       (s16)(receive_buffer[1] | (receive_buffer[2] << 8));
      netplay.remote_stalls_report = receive_buffer[3];
      receive_bytes -= 4;
      receive_buffer += 4;
      break;

    case NETPLAY_PACKET_TYPE_SET_LATENCY:
      receive_packet_ensure_available(4);

      netplay.frame_latency_update_at_frame =
       receive_buffer[1] | (receive_buffer[2] << 8);
      netplay.frame_latency_update_to = receive_buffer[3];
      receive_bytes -= 4;
      receive_buffer += 4;
      break;

    case NETPLAY_PACKET_TYPE_PAUSE:
      if((netplay.pause == 0) ||
       (netplay.pause_type != NETPLAY_PAUSE_RECEIVING_SAVESTATE))
      {
        status_message_raw("Remote-side paused netplay.");
        copy_screen_half_intensity(get_screen_ptr());
        update_status_message();
        audio_pause();
   
        netplay.pause = 1;
        netplay.pause_type = NETPLAY_PAUSE_MENU;
      }
      receive_bytes--;
      receive_buffer++;
      break;

    case NETPLAY_PACKET_TYPE_UNPAUSE:
      if((netplay.pause == 0) ||
       (netplay.pause_type != NETPLAY_PAUSE_RECEIVING_SAVESTATE))
      {
        status_message_raw("Remote-side unpaused netplay.");
        netplay.pause = 0;
        audio_unpause();
      }

      receive_bytes--;
      receive_buffer++;
      break;

    case NETPLAY_PACKET_TYPE_SAVESTATE_START:
    {
      receive_packet_ensure_available(4);

      netplay.savestate_receive_size =
       receive_buffer[1] | (receive_buffer[2] << 8) | (receive_buffer[3] << 16);

      netplay.savestate_receive_buffer = malloc(netplay.savestate_receive_size);
      netplay.savestate_receive_ptr = netplay.savestate_receive_buffer;
      netplay.savestate_receive_bytes_remaining =
       netplay.savestate_receive_size;

      printf("Got savestate start packet (%d bytes).\n",
       netplay.savestate_receive_size);

      netplay_reset_buffers();

      status_message("Remotely loading state (%d bytes)",
       netplay.savestate_receive_bytes_remaining);

      send_savestate_block_confirm(0);

      copy_screen_half_intensity(get_screen_ptr());
      update_status_message();
      audio_pause();

      netplay.pause_type = NETPLAY_PAUSE_RECEIVING_SAVESTATE;
      netplay.pause = 1;

      receive_bytes -= 4;
      receive_buffer += 4;
      break;
    }

    case NETPLAY_PACKET_TYPE_SAVESTATE_BLOCK:
    {
      receive_packet_ensure_available(5);

      u32 block_size = receive_buffer[1] | (receive_buffer[2] << 8);
      u32 block_number = receive_buffer[3] | (receive_buffer[4] << 8);

      receive_packet_ensure_available(block_size + 5);

      memcpy(netplay.savestate_receive_ptr, receive_buffer + 5,
       block_size);

      netplay.savestate_receive_ptr += block_size;
      netplay.savestate_receive_bytes_remaining -= block_size;

      send_savestate_block_confirm(block_number + 1);

      if(netplay.savestate_receive_bytes_remaining == 0)
      {
        load_state(NULL, netplay.savestate_receive_buffer,
         netplay.savestate_receive_size);

        free(netplay.savestate_receive_buffer);

        netplay.pause = 0;
        audio_unpause();
      }

      receive_bytes -= block_size + 5;
      receive_buffer += block_size + 5;
      break;
    }

    case NETPLAY_PACKET_TYPE_SAVESTATE_BLOCK_CONFIRM:
      receive_packet_ensure_available(3);

      netplay.savestate_send_block_confirmed =
       receive_buffer[1] | (receive_buffer[2] << 8);

      receive_bytes -= 3;
      receive_buffer += 3;
      break;

    default:
      printf("Got %d bytes of unexpected stuff.\n", receive_bytes);
      fflush(stdout);

      receive_bytes--;
      receive_buffer++;
      break;      
  }

  netplay.receive_bytes_remaining = receive_bytes;
  netplay.receive_buffer_ptr = receive_buffer;

  return message_type;
}


s32 queue_controls_remote(void)
{
  control_buffer_entry_struct remote_controls;
  u32 local_frame_number = netplay.frame_number;
  s32 newest_remote_frame_number = -1;
  s32 new_packet_type;
  s32 accepted_packet_type = -1;
  
  while(1)
  {
    new_packet_type = receive_packet(&remote_controls);

    if(new_packet_type == NETPLAY_PACKET_TYPE_CONTROLS)
    {
      accepted_packet_type = new_packet_type;
      queue_controls(&(netplay.control_buffer_remote),
       remote_controls.frame_number, remote_controls.controls);
      newest_remote_frame_number = remote_controls.frame_number;
    }
    else
    {
      if(new_packet_type == -1)
        break;

      if(new_packet_type == -2)
      {
        netplay_disconnect();

        status_message_raw("Lost netplay connection.\n");
        return -2;
      }

      accepted_packet_type = new_packet_type;
    }
  }

  if(newest_remote_frame_number != -1)
  {
    s32 frame_latency =
     (s16)(local_frame_number - newest_remote_frame_number);
    netplay.total_latency += frame_latency;
  }

  return accepted_packet_type;
}

#ifndef SOL_TCP
#define SOL_TCP IPPROTO_TCP
#endif

void disable_nagle(void)
{
#ifdef WIN32_BUILD
  char nagle_options = 1;
#else
  u32 nagle_options = 1;
#endif
  setsockopt(netplay.socket_handle, SOL_TCP, TCP_NODELAY, &nagle_options,
   sizeof(u32));
}

void tcp_quick_ack(void)
{
#ifndef WIN32_BUILD
  u32 quick_ack_options = 1;
  setsockopt(netplay.socket_handle, SOL_TCP, TCP_QUICKACK, &quick_ack_options,
   sizeof(u32));
#endif
}

void netplay_connect(void)
{
#ifdef WIN32_BUILD
  static u32 initialized = 0;
  if(initialized == 0)
  {
    WSADATA info;
    WSAStartup(MAKEWORD(1, 1), &info);
    initialized = 1;
  }
#endif

  if(config.netplay_type != NETPLAY_TYPE_NONE)
  {
    netplay.frame_number = 0;

    if(netplay.active)
      netplay_disconnect();

    reset_control_buffer(&(netplay.control_buffer_local));
    reset_control_buffer(&(netplay.control_buffer_remote));

    netplay.frame_latency_update_at_frame = 0x10000;

    netplay.total_latency = 0;
    netplay.latency_calculation_frames = 0;
    netplay.stalls = 0;

    netplay.receive_bytes_remaining = 0;
    netplay.receive_buffer_ptr = netplay.receive_buffer;

    netplay.send_control_buffer_limit = 0;
    netplay.send_control_packets_buffered = 0;
    netplay.send_buffer_bytes = 0;
    netplay.send_buffer_ptr = netplay.send_buffer;

    if(config.netplay_type == NETPLAY_TYPE_CLIENT)
    {
      if(netplay_connect_client() >= 0)
        netplay.pause = 0;
    }
    else
    {
      if(netplay_connect_server() >= 0)
      {
        clear_screen();
        status_message_raw("Waiting for connection.");
        update_status_message();
        audio_pause();
      
        netplay.pause = 1;
        netplay.pause_type = NETPLAY_PAUSE_ACCEPTING_CONNECTION;
      }
    }

    netplay_queue_latency_filler();

    disable_blocking();
    disable_nagle();
    tcp_quick_ack();
  }
  else
  {
    if(netplay.active)
      netplay_disconnect();

    if(netplay.pause)
      audio_unpause();

    netplay.pause = 0;
    netplay.can_send = 0;
    netplay.active = 0;
  }
}

void netplay_frame_update(u32 new_local_controls, u32 *_use_local_controls,
 u32 *_use_remote_controls)
{
  control_buffer_entry_struct *local_controls;
  control_buffer_entry_struct *remote_controls;
  control_buffer_struct *control_buffer_local = &(netplay.control_buffer_local);
  control_buffer_struct *control_buffer_remote =
   &(netplay.control_buffer_remote);
  u32 frame_number = netplay.frame_number;
  u32 frame_latency = netplay.frame_latency;
  u32 back_frame;

  if(netplay.pause)
  {
    control_buffer_entry_struct _remote_controls;

    switch(netplay.pause_type)
    {
      case NETPLAY_PAUSE_ACCEPTING_CONNECTION:
        if(server_wait_for_connection() >= 0)
        {
          send_set_latency(0, config.netplay_server_frame_latency);
          netplay.pause = 0;
          netplay.can_send = 1;
          audio_unpause();
        }
        break;

      case NETPLAY_PAUSE_MENU:
      case NETPLAY_PAUSE_RECEIVING_SAVESTATE:
        receive_packet(&_remote_controls);
        break;

      case NETPLAY_PAUSE_RECEIVE_STALL:
      {
        s32 queue_controls_response = queue_controls_remote();
        if(queue_controls_response > 0)
        {
          netplay.pause = 0;
          audio_unpause();
        }
        break;
      }

      case NETPLAY_PAUSE_SENDING_SAVESTATE:
      {
        u32 block_size = netplay.savestate_send_bytes_remaining;

        // Receive confirmations
        receive_packet(&_remote_controls);

        if(netplay.savestate_send_block ==
         netplay.savestate_send_block_confirmed)
        {
          if(block_size)
          {
            if(block_size > NETPLAY_SAVESTATE_BLOCK_SIZE)
              block_size = NETPLAY_SAVESTATE_BLOCK_SIZE;

            send_savestate_block(netplay.savestate_send_ptr, block_size,
             netplay.savestate_send_block);
            netplay.savestate_send_ptr += block_size;
            netplay.savestate_send_block++;

            netplay.savestate_send_bytes_remaining -= block_size;
            if(netplay.savestate_send_bytes_remaining == 0)
            {
              free(netplay.savestate_send_buffer);
              netplay.savestate_send_buffer = NULL;
            }
          }
          else
          {
            netplay.pause = 0;
            audio_unpause();
          }
        }
        break;
      }
    }

    return;
  }

  send_controls(new_local_controls, frame_number);
  back_frame = (frame_number - frame_latency) & 0xFFFF;

  if(frame_number == netplay.frame_latency_update_at_frame)
  {
    status_message("Updating latency to %d.", netplay.frame_latency_update_to);
    netplay.frame_latency = netplay.frame_latency_update_to;
    netplay.frame_latency_update_at_frame = 0x10000;
  }

  // Update new local controls
  queue_controls(&(netplay.control_buffer_local), frame_number,
   new_local_controls);

  // Update new remote controls (may update multiple)
  if(queue_controls_remote() == -2)
    return;

  if(netplay.pause == 1)
    return;
 
  local_controls =
   find_controls_at_frame_number(control_buffer_local, back_frame);
  remote_controls =
   find_controls_at_frame_number(control_buffer_remote, back_frame);

  if(local_controls == NULL)
  {
    printf("wtf? We're on frame %d but don't have controls for frame %d??\n",
     frame_number, back_frame);
    exit(-1);
  }

  if(remote_controls == NULL)
  {
    u32 tries = 0;
    (netplay.stalls)++;

    flush_send_buffer();

    while(remote_controls == NULL)
    {
      if((queue_controls_remote() == -2) || netplay.pause)
        return;

      remote_controls =
       find_controls_at_frame_number(control_buffer_remote,
       back_frame);

      delay_us(1000);

      tries++;
      if(tries == 6000)
      {
        status_message_raw("Remote-side is stalling. Pausing.");
        copy_screen_half_intensity(get_screen_ptr());
        update_status_message();
        audio_pause();

        netplay.pause = 1;
        netplay.pause_type = NETPLAY_PAUSE_RECEIVE_STALL;
        return;
      }
    }
  }

  *_use_local_controls = local_controls->controls;
  *_use_remote_controls = remote_controls->controls;
      
  netplay.frame_number = (frame_number + 1) & 0xFFFF;
  (netplay.latency_calculation_frames)++;

  if(netplay.latency_calculation_frames == LATENCY_CALCULATION_PERIOD)
  {
    netplay.period_latency = netplay.total_latency;
    netplay.period_stalls = netplay.stalls;

    netplay.total_latency = 0;
    netplay.stalls = 0;
    netplay.latency_calculation_frames = 0;

    send_latency_report(netplay.period_latency, netplay.period_stalls);
  }
}

s32 netplay_send_savestate(char *file_name)
{
  if(netplay.can_send && (netplay.pause == 0))
  {
    printf("Sending savestate to other side.\n");

    netplay.savestate_send_buffer =
     preload_state(file_name, &netplay.savestate_send_bytes_remaining, 1);
    netplay.savestate_send_ptr = netplay.savestate_send_buffer;

    flush_receive();
    netplay_reset_buffers();

    printf("Got state size %d bytes.\n", 
     netplay.savestate_send_bytes_remaining);

    netplay.savestate_send_block = 0;
    netplay.savestate_send_block_confirmed = -1;

    printf("Sending savestate start packet.\n");
    send_savestate_start(netplay.savestate_send_bytes_remaining);

    printf("Pausing...\n");
    netplay.pause = 1;
    netplay.pause_type = NETPLAY_PAUSE_SENDING_SAVESTATE;
    audio_pause();

    status_message("Sending savestate %s", file_name);
    copy_screen_half_intensity(get_screen_ptr());
    update_status_message();
  }
  return 0;
}

