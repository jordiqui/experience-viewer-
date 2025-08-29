import { Chess } from 'chess.js';
import { Chessboard } from 'cm-chessboard';

const chess = new Chess();

const board = new Chessboard(document.getElementById('board'), {
  position: chess.fen(),
  orientation: 'white',
  showNotation: true,
});
