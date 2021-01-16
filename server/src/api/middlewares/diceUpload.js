import multer from 'multer';

const storage = multer.diskStorage({
  destination(request, file, callback) {
    callback(null, './uploads/dices');
  },
  filename(request, file, callback) {
    callback(null, `${file.fieldname}-${Date.now()}.jpg`);
  },
});

export default multer({
  storage,
});
