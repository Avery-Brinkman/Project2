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
        # users = self.socket.recv(1024).decode()
        # print("List of users in the group: ", users)

        # Start a separate thread to receive messages from the server
        receive_thread = threading.Thread(target=self.receive_messages)
        receive_thread.start()

        # Allow the user to enter a command
        while True:
            print(" 'quit' - disconnect from server")
            print(" 'join' - join the public group")
            print(" 'exit' - leave the public group ")
            print(" 'usrs' - Get users in the public group ")
            print(" 'post' - Post a message in the public group ")
            print(" 'mesg' - Get a message from the public group ")
            print(" 'grps' - Get the list of groups available to join ")
            print(" 'join <group_id>' - join a group ")
            print(" 'exit <group_id>' - exit a group ")
            print(" 'usrs <group_id>' - Get the users in a group")
            print(" 'post <group_id>' - Post a message to a group")
            print(" 'get <group_id>' - Get a message from a group")
            message = input("Enter a command: ")

            if message == 'quit':
                exit
            elif message == 'exit':
                self.socket.send(b"%exit\n")
                print(self.socket.recv(1024).decode())
                self.socket.close()
                print("You have left the group.")
                time.sleep(1)
                exit()
                break
            elif message == 'join':
                self.socket.send(b"%join\n")
                print(self.socket.recv(1024).decode())
                break
            elif message == 'usrs':
                self.socket.send(b"%usrs\n")
                print(self.socket.recv(1024).decode())
                break
            elif message == 'post':
                messageToPost = input("What message would you like to post? ")
                self.socket.send(b"%mesg\n")
                print(self.socket.recv(1024).decode())
                break
            elif message == 'mesg':
                self.socket.send(b"%mesg\n")
                print(self.socket.recv(1024).decode())
                break
            elif message == 'grps':
                self.socket.send(b"%grps\n")
                print(self.socket.recv(1024).decode())
                
                break
            else: 
                print("Invalid command. Please try again.")




    def receive_messages(self):
        # Continuously receive messages from the server
        self.socket.send(b"%mesg\n")
        print(self.socket.recv(1024).decode())
        

    def receive_groups(self):
        self.socket.send(b"%grps\n")
        print(self.socket.recv(1024).decode())
        # Continuously receive messages from the server
        while True:
            data = self.socket.recv(1024).decode()
            if data:
                print(data)
            else:
                print("Disconnected from the server.")
                break

    def receive_users(self):
        # continuously receive users from server
        while True:
            data = self.socket.recv(1024).decode()
            if data:
                print(data)
            else:
                print("Disconnected from the server.")
                break

    def leave_group(self):
        # Send leave command to the server
        self.socket.sendall("%exit\n".encode())

        # Close the socket and terminate the client
        

# Main function to run the client program
def main():
    client = Client()
    client.connect_to_server()
    client.join_group()

if __name__ == '__main__':
    main()
