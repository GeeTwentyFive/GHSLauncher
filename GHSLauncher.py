import miniupnpc
import socket
import os
import time
import win32gui
import tkinter as tk


VIRTUAL_ADAPTER_IPv4 = "10.10.10.10"
PORT = 27015
HIDDEN_SOURCE_WINDOW_NAME = "HIDDEN: SOURCE - Enhanced Edition"


# TODO: chdir to sourcemods\hidden

try:
        upnp = miniupnpc.UPnP()
        if not upnp.discover():
                raise RuntimeError("No UPnP devices found")
        upnp.selectigd()
        upnp.addportmapping(
		27015,
		"UDP",
		upnp.lanaddr,
		27015,
		"Source",
		""
	)
        upnp.addportmapping(
		27015,
		"TCP",
		upnp.lanaddr,
		27015,
		"Source",
		""
	)
except Exception: pass


def connect(ip: str):
	os.system("explorer steam://rungameid/9826266959967158487")

	# Terminate existing IPv4Tunnel daemon if one is running, via TCP IPC
	with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
		s.settimeout(0.1)
		try: s.connect(("localhost", 55555))
		except TimeoutError: pass

	os.system(".\\IPv4Tunnel.exe {PORT} {ip}") # TODO

	hidden_hwnd = 0
	while not hidden_hwnd:
		hidden_hwnd = win32gui.FindWindow(None, HIDDEN_SOURCE_WINDOW_NAME)
		time.sleep(0.1)

	win32gui.SetWindowLong(hidden_hwnd, -16,
		win32gui.GetWindowLong(hidden_hwnd, -16) & ~(
			0x00800000 |	# WS_BORDER
			0x00400000 |	# WS_DLGFRAME
			0x00010000 |	# WS_MAXIMIZEBOX
			0x00020000 |	# WS_MINIMIZEBOX
			0x00080000 |	# WS_SYSMENU
			0x00040000	# WS_THICKFRAME
		)
	)
	win32gui.SetWindowLong(hidden_hwnd, -20,
		win32gui.GetWindowLong(hidden_hwnd, -20) & ~(
			0x00000001 |	# WS_EX_DLGMODALFRAME
			0x02000000 |	# WS_EX_COMPOSITED
			0x00000100 |	# WS_EX_WINDOWEDGE
			0x00000200 |	# WS_EX_CLIENTEDGE
			0x00080000 |	# WS_EX_LAYERED
			0x00020000 |	# WS_EX_STATICEDGE
			0x00000080 |	# WS_EX_TOOLWINDOW
			0x00040000	# WS_EX_APPWINDOW
		)
	)
	win32gui.SetWindowPos(hidden_hwnd, None, 0, 0, 0, 0,
		(
			0x0020 |	# SWP_FRAMECHANGED
			0x0040 |	# SWP_SHOWWINDOW
			0x0001 |	# SWP_NOSIZE
			0x0002 |	# SWP_NOMOVE
			0x0400		# SWP_NOSENDCHANGING
		)
	)
	win32gui.ShowWindow(hidden_hwnd, 3) # SW_SHOWMAXIMIZED
        
	os.system(f"explorer steam://connect/{VIRTUAL_ADAPTER_IPv4}:{PORT}") # TODO

def host_server():
        os.system(f"start .\\ServerFiles\\srcds.exe -game hidden -tickrate 128 -ip {VIRTUAL_ADAPTER_IPv4} -port {PORT}")
        connect("")


root = tk.Tk()
root.title("GHSLauncher")
root.resizable(False, False)
root.geometry("250x75")
root.eval("tk::PlaceWindow . center")

remote_ip_entry = tk.Entry(root, width=39)
remote_ip_entry.pack(side=tk.TOP)
tk.Button(
        root,
        text="Connect",
        command=lambda: connect(remote_ip_entry.get())
).pack(side=tk.TOP)

tk.Button(
        root,
        text="Host server",
        command=host_server
).pack(side=tk.BOTTOM)

root.mainloop()