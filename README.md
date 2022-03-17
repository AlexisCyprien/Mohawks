# Mohawks HTTP 1.0 Server

Mohawks is a simple HTTP/1.0 server for linux written in C.

It was made as an academic project for University of Rouen, Normandy, France

## Install & Usage

On the linux distribution of your choice open a terminal and type:
```bash
git clone https://github.com/AlexisCyprien/Mohawks.git 
cd Mohawks
make
sudo ./mohawks
```

1. Open your favorite browser
2. Type the following in the URL bar : "localhost" 
3. Press Enter
4. Et voilà !

You will first be greated with an ***awesome*** home page, feel free to explore it and test how well the server works.
![home1](https://user-images.githubusercontent.com/90419469/158907200-b9852b2c-ba76-44b0-aad3-5da191d7604d.png)

To display a content of your choice, simply drop the desired files in the `content/` directory.
Because Mohawks is designed as a web server, all `index.html` files will be automatically displayed when their containing folder is requested.

## File Explorer

We have implemented a basic file explorer in the server, it allows you to browse through the `content/` directory and open all types of files and folders.

The operating is simple: if an `index.html` file is located in the requested folder, it will be delivered. Otherwise, the file explorer will index all entries contained in it and display them in a *nice and fresh* presentation including their last modification date and size.

Just click on a link to access it or on the curved left arrow to go to the previous directory.
![file_exp](https://user-images.githubusercontent.com/90419469/158908192-5a5d27f5-145a-49d4-b338-6c4a814aa283.png)


## Contributors

[Alexis CYPRIEN](https://github.com/AlexisCyprien) and [Edouard ROUCH](https://github.com/EdouardRouch).
 
