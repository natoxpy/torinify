PRAGMA foreign_keys = ON;

create TABLE MediaSource (
    "id" INTEGER PRIMARY KEY,
    "path" TEXT NOT NULL UNIQUE
);

CREATE TABLE Album (
    "id" INTEGER PRIMARY KEY,
    "title" TEXT NOT NULL,
    "artist" TEXT,
    "year" INT
);

CREATE TABLE Metadata (
    "id" INTEGER PRIMARY KEY,
    "artist" TEXT,
    "year" INT,
    "genre" INT
);

CREATE TABLE Music (
    "id" INTEGER PRIMARY KEY,
    "title" TEXT NOT NULL,
    "fullpath" TEXT NOT NULL UNIQUE,
    "source" INTEGER, 
    "album" INTEGER,
    "metadata" INTEGER NOT NULL, 
    FOREIGN KEY (source) REFERENCES MediaSource(id) ON DELETE SET NULL,
    FOREIGN KEY (album) REFERENCES Album(id) ON DELETE SET NULL,
    FOREIGN KEY (metadata) REFERENCES Metadata(id) ON DELETE CASCADE 
);

CREATE TABLE MusicAltTitle (
    "id" INTEGER PRIMARY KEY,
    "title" TEXT NOT NULL,
    "music_id" INTEGER NOT NULL,
    "language" TEXT NOT NULL,
    FOREIGN KEY (music_id) REFERENCES Music(id) ON DELETE CASCADE
);
