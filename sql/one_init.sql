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

CREATE VIRTUAL TABLE MusicFTS USING fts5(title, album, content='Music', content_rowid='id');


-- Insert trigger
CREATE TRIGGER MusicFTS_insert AFTER INSERT ON Music BEGIN
    INSERT INTO 
        MusicFTS (rowid, title, album) 
    VALUES (NEW.id, NEW.title, (SELECT title from Album WHERE id = NEW.album));
END;

-- Update trigger
CREATE TRIGGER MusicFTS_update AFTER UPDATE ON Music BEGIN
    UPDATE MusicFTS SET 
        title = NEW.title,
        album = (select title from Album WHERE id = NEW.album)
    WHERE rowid = NEW.id;
END;

-- Delete trigger
CREATE TRIGGER MusicFTS_delete AFTER DELETE ON Music BEGIN
    DELETE FROM MusicFTS WHERE rowid = OLD.id;
END;

-- -- Trigger to insert data into the FTS table when a new music entry is added
-- CREATE TRIGGER Music_insert_trigger AFTER INSERT ON Music
-- BEGIN
--   INSERT INTO Music_FTS(rowid, title) VALUES (new.id, new.title);
-- END;
-- 
-- -- Trigger to update the FTS table when a music entry's title is updated
-- CREATE TRIGGER Music_update_trigger AFTER UPDATE ON Music
-- BEGIN
--   UPDATE Music_FTS SET title = new.title WHERE rowid = old.id;
-- END;
-- 
-- -- Trigger to delete from the FTS table when a music entry is deleted
-- CREATE TRIGGER Music_delete_trigger AFTER DELETE ON Music
-- BEGIN
--   DELETE FROM Music_FTS WHERE rowid = old.id;
-- END;
