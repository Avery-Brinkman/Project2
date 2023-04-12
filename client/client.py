#!/usr/bin/env python3

import socket
import threading 
import time

# define server address and port
ipAddr = "127.0.0.1" # localhost
port = 5000

class Client:
    def __init__(self):
        self.username = None
        self.socket = None 

    def connect_to_server(self):
        # Connect to the server
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.connect((ipAddr, port))

    def join_group(self):
        # Prompt the user to enter a username
        while True:
            self.username = input("Enter a username to join the group: ")
            if self.username:
                break
            else:
                print("Username cannot be empty. Please try again.")
        
        # Send the username to the server to join the group
        self.socket.sendall(self.username.encode())
        # Receive the list of existing users from the server
        users = self.socket.recv(1024).decode()
        print("List of users in the group: ", users)

        # Start a separate thread to receive messages from the server
        receive_thread = threading.Thread(target=self.receive_messages)
        receive_thread.start()

        # Allow the user to send messages or leave the group
        while True:
            message = input("Enter a message to send or type 'leave' to exit the group: ")
            if message == 'leave':
                self.leave_group()
                break
            else:
                self.send_message(message)

    def send_message(self, message):
            # Send the message to the server
            self.socket.sendall(message.encode())

    def receive_messages(self):
        # Continuously receive messages from the server
        while True:
            data = self.socket.recv(1024).decode()
            if data:
                print(data)
            else:
                print("Disconnected from the server.")
                break

    def leave_group(self):
        # Send leave command to the server
        self.socket.sendall("leave".encode())

        # Close the socket and terminate the client
        self.socket.close()
        print("You have left the group.")
        time.sleep(1)
        exit()

# Main function to run the client program
def main():
    client = Client()
    client.connect_to_server()
    client.join_group()

if __name__ == '__main__':
    main()
