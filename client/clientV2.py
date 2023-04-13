#!/usr/bin/env python3

import socket
import threading 
import time

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

        # Start a separate thread to receive messages from the server
            receive_thread = threading.Thread(target=self.receive_messages)
            receive_thread.start()

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
                    self.socket.send(b"%quit\n")
                    print(self.socket.recv(1024).decode())

                elif message == 'exit':
                    self.socket.send(b"%exit\n")
                    print(self.socket.recv(1024).decode())
                    self.socket.close()
                    print("You have left the group.")
                    time.sleep(1)
                    exit()

                elif message == 'join':
                    groupNum = int(input("What group do you want to join? "))
                    self.socket.send(b"%join\n", groupNum)
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
        

# Main function to run the client program
def main():
    client = Client()
    client.connect_to_server()
    client.join_group()

if __name__ == '__main__':
    main()