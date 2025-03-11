PRAGMA foreign_keys = ON;

CREATE TABLE "MediaSource" (
    "id" INTEGER PRIMARY KEY,
    "path" TEXT NOT NULL UNIQUE
);

-- ref `AlbumAltTitle`
-- ref `AlbumArtists`
CREATE TABLE Album (
    "id" INTEGER PRIMARY KEY,
    "title" TEXT NOT NULL,
    "year" TEXT -- ISO8601 (year)-(mo)-(da)T
);

-- ref `AlbumArtists`
-- ref `MetadataArtists`
-- ref `ArtistAltTitle`
CREATE TABLE Artist (
    "id" INTEGER PRIMARY KEY,
    "name" TEXT NOT NULL
);

CREATE TABLE "AlbumArtists" (
    "id" INTEGER PRIMARY KEY,
    "album_id" INTEGER NOT NULL,
    "artist_id" INTEGER NOT NULL,
    -- 'Main Artist', 'Featured Artist'
    "artist_type" TEXT NOT NULL,
    FOREIGN KEY ("album_id") REFERENCES "Album" ("id") ON DELETE CASCADE,
    FOREIGN KEY ("artist_id") REFERENCES "Artist" ("id") ON DELETE CASCADE
);

-- ref `MetadataGenres`
CREATE TABLE Genre (
    "id" INTEGER PRIMARY KEY,
    "genre" TEXT NOT NULL
);

-- ref `MetadataArtists`
-- ref `MetadataGenres`
CREATE TABLE Metadata (
    "id" INTEGER PRIMARY KEY,
    "year" TEXT -- ISO8601 (year)-(mo)-(da)T
);

-- CREATE TABLE "MetadataArtists" (
--     "id" INTEGER PRIMARY KEY,
--     "metadata_id" INTEGER NOT NULL,
--     "artist_id" INTEGER NOT NULL,
--     FOREIGN KEY ("metadata_id") REFERENCES "Metadata" ("id") ON DELETE CASCADE,
--     FOREIGN KEY ("artist_id") REFERENCES "Artist" ("id") ON DELETE CASCADE
-- );

CREATE TABLE "MetadataGenres" (
    "id" INTEGER PRIMARY KEY,
    "metadata_id" INTEGER NOT NULL,
    "genre_id" INTEGER NOT NULL,
    FOREIGN KEY ("metadata_id") REFERENCES "Metadata" ("id") ON DELETE CASCADE,
    FOREIGN KEY ("genre_id") REFERENCES "Genre" ("id") ON DELETE CASCADE
);

-- ref `MusicAltTitle`
CREATE TABLE Music (
    "id" INTEGER PRIMARY KEY,
    "title" TEXT NOT NULL,
    "fullpath" TEXT NOT NULL UNIQUE,
    "source_id" INTEGER,
    "metadata_id" INTEGER NOT NULL,
    FOREIGN KEY ("source_id") REFERENCES "MediaSource" ("id") ON DELETE SET NULL,
    FOREIGN KEY ("metadata_id") REFERENCES "Metadata" ("id") ON DELETE CASCADE
);

-- Many Too Many
CREATE TABLE "MusicAltTitle" (
    "id" INTEGER PRIMARY KEY,
    "music_id" INTEGER NOT NULL,
    "title_id" INTEGER NOT NULL,
    FOREIGN KEY ("music_id") REFERENCES "Music" ("id") ON DELETE CASCADE,
    FOREIGN KEY ("title_id") REFERENCES "AlternativeName" ("id")
    ON DELETE CASCADE
);
-- Many Too Many
CREATE TABLE "AlbumMusics" (
    "id" INTEGER PRIMARY KEY,
    "music_id" INTEGER NOT NULL,
    "album_id" INTEGER NOT NULL,
    FOREIGN KEY ("music_id") REFERENCES "Music" ("id") ON DELETE CASCADE,
    FOREIGN KEY ("album_id") REFERENCES "Album" ("id")
    ON DELETE CASCADE
);

-- Many Too Many
CREATE TABLE "ArtistAltTitle" (
    "id" INTEGER PRIMARY KEY,
    "artist_id" INTEGER NOT NULL,
    "title_id" INTEGER NOT NULL,
    FOREIGN KEY ("artist_id") REFERENCES "Artist" ("id") ON DELETE CASCADE,
    FOREIGN KEY ("title_id") REFERENCES "AlternativeName" ("id")
    ON DELETE CASCADE
);

CREATE TABLE "AlbumAltTitle" (
    "id" INTEGER PRIMARY KEY,
    "album_id" INTEGER NOT NULL,
    "title_id" INTEGER NOT NULL,
    FOREIGN KEY ("album_id") REFERENCES "Album" ("id") ON DELETE CASCADE,
    FOREIGN KEY ("title_id") REFERENCES "AlternativeName" ("id") ON DELETE CASCADE
);

-- ref `AlbumAltTitle`
-- ref `ArtistAltTitle`
-- ref `MusicAltTitle`
CREATE TABLE "AlternativeName" (
    "id" INTEGER PRIMARY KEY,
    "title" TEXT NOT NULL,
    -- ISO 639
    "language" TEXT NOT NULL
);
